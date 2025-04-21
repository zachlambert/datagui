#pragma once

#include "datagui/exception.hpp"
#include "datagui/tree/state.hpp"
#include "datagui/tree/unique_any.hpp"
#include "datagui/tree/vector_map.hpp"
#include <assert.h>
#include <functional>
#include <stack>
#include <string>

namespace datagui {

enum class ElementType;
#if 0
enum class ElementType {
  HorizontalLayout,
  VerticalLayout,
  TextBox,
  TextInput,
  Button
};
constexpr std::size_t ElementTypeCount = 5;
#endif

class ElementContainer {
public:
  virtual int emplace() = 0;
  virtual void pop(int index) = 0;
};

template <typename Data>
requires std::is_default_constructible_v<Data>
class ElementContainerImpl : public ElementContainer {
public:
  int emplace() override final {
    return datas.emplace();
  }
  void pop(int index) override final {
    datas.pop(index);
  }
  Data& get(int index) {
    return datas[index];
  }
  const Data& get(int index) const {
    return datas[index];
  }

protected:
  VectorMap<Data> datas;
};

struct ElementNode {
  ElementType type;
  int data_index;
  State state;

  std::string key = "";
  bool is_new = true;
  bool rerender = true;
  bool revisit = true;
  bool visible = true;
  bool retain = false;

  int parent = -1;
  int prev = -1;
  int next = -1;
  int first_child = -1;
  int last_child = -1;
  int first_variable = -1;

  int version = 0; // Debug information

  ElementNode(ElementType type, int data_index) :
      type(type), data_index(data_index) {}
};

struct VariableNode {
  int element;

  UniqueAny data;
  bool modified = false;

  // Linked list of data nodes for a given gui node
  int prev = -1;
  int next = -1;

  VariableNode(int element) : element(element) {}
};

// Forward declare Element_

template <bool IsConst>
class Element_;
using Element = Element_<false>;
using ConstElement = Element_<true>;

// Forward declare Variable_

template <typename T, bool IsConst>
class Variable_;
template <typename T>
using Var = Variable_<T, false>;
template <typename T>
using ConstVar = Variable_<T, true>;

class Tree {
  template <bool IsConst>
  friend class NodeElement_;
  template <typename T, bool IsConst>
  friend class Variable_;

public:
  Tree() {}

  void begin();
  void end();

  void next(ElementType type, const std::string& key = "");
  bool down_if();
  void down();
  void up();

  Element root();
  ConstElement root() const;
  Element current();
  ConstElement current() const;

  template <typename T>
  Var<T> variable(const std::function<T()>& construct = []() { return T(); });

  template <typename Data>
  void register_element_type(const ElementType type) {
    assert(!element_containers[int(type)]);
    if (element_containers.size() <= int(type)) {
      element_containers.resize(int(type) + 1);
    }
    element_containers[int(type)] =
        std::make_unique<ElementContainerImpl<Data>>();
  }

private:
  int create_element(int parent, int prev, ElementType type);
  void rerender_element(int node, ElementType type);
  void remove_element(int node);

  int create_variable(int element);
  void remove_variable(int variable);

  void variable_mutate(int variable);
  void set_revisit(int node);
  void set_rerender(int node);

  std::string element_debug(int element) const;

  std::vector<std::unique_ptr<ElementContainer>> element_containers;
  VectorMap<ElementNode> elements;
  VectorMap<VariableNode> variables;

  bool is_new = true;
  int root_ = -1;
  int parent_ = -1;
  int current_ = -1;
  int variable_current_ = -1;
  std::stack<int> variable_stack_;
  std::vector<int> queue_revisit_;
  std::vector<int> queue_rerender_;
  std::vector<int> queue_remove_;
};

template <typename T, bool IsConst>
class Variable_ {
  using data_ptr_t = std::conditional_t<IsConst, const T*, T*>;
  using data_ref_t = std::conditional_t<IsConst, const T&, T&>;

public:
  data_ref_t operator*() const {
    return *data_ptr;
  }
  data_ptr_t operator->() const {
    return data_ptr;
  }
  T& mut() const {
    static_assert(!IsConst);
    tree->variable_mutate(variable);
    return *data_ptr;
  }
  bool modified() const {
    return tree->variables[variable].modified;
  }

  template <
      bool OtherConst,
      typename = std::enable_if_t<IsConst || !OtherConst>>
  Variable_(const Variable_<T, OtherConst>& other) :
      tree(other.tree), variable(other.variable), data_ptr(other.data_ptr) {}

  template <
      bool OtherConst,
      typename = std::enable_if_t<IsConst || !OtherConst>>
  Variable_(Variable_<T, OtherConst>&& other) :
      tree(other.tree), variable(other.variable), data_ptr(other.data_ptr) {}

  Variable_() : tree(nullptr), variable(-1), data_ptr(nullptr) {}

  operator bool() const {
    return tree;
  }

private:
  Variable_(Tree* tree, int variable) :
      tree(tree),
      variable(variable),
      data_ptr(tree->variables[variable].data.cast<T>()) {}

  Tree* tree;
  int variable;
  data_ptr_t data_ptr;

  friend class Tree;
};
template <typename T>
using Variable = Variable_<T, false>;
template <typename T>
using ConstVariable = Variable_<T, true>;

template <bool IsConst>
class Element_ {
  using tree_ptr_t = std::conditional_t<IsConst, const Tree*, Tree*>;
  using state_ptr_t = std::conditional_t<IsConst, const State*, State*>;
  using state_ref_t = std::conditional_t<IsConst, const State&, State&>;

public:
  Element_ first_child() const {
    assert(index != -1);
    return NodeElement_(tree, tree->nodes[index].first_child);
  }
  Element_ last_child() const {
    assert(index != -1);
    return NodeElement_(tree, tree->nodes[index].last_child);
  }
  Element_ next() const {
    assert(index != -1);
    return NodeElement_(tree, tree->nodes[index].next);
  }
  Element_ prev() const {
    assert(index != -1);
    return NodeElement_(tree, tree->nodes[index].prev);
  }
  Element_ parent() const {
    assert(index != -1);
    return NodeElement_(tree, tree->nodes[index].parent);
  }

  state_ref_t operator*() const {
    return tree->nodes[index].state;
  }
  state_ptr_t operator->() const {
    return &tree->nodes[index].state;
  }

  template <typename Data>
  std::conditional_t<IsConst, const Data&, Data&> data() const {
    const auto& node = tree->nodes[index];
    auto container = tree->element_containers[int(node.type)];
    auto container_cast = dynamic_cast<ElementContainerImpl<Data>>(container);
    assert(container_cast);
    return container_cast->get(node.element_index);
  }

  template <typename Data>
  std::conditional_t<IsConst, const Data&, Data&> data_if() const {
    const auto& node = tree->nodes[index];
    auto container = tree->element_containers[int(node.type)];
    auto container_cast = dynamic_cast<ElementContainerImpl<Data>>(container);
    if (!container_cast) {
      return nullptr;
    }
    return &container_cast->get(node.element_index);
  }

  bool visible() const {
    return tree->nodes[index].visible;
  }
  std::string debug() const {
    return tree->node_debug(index);
  }

  void trigger() const {
    tree->queue_revisit_.emplace_back(index);
  }

  template <
      bool OtherConst,
      typename = std::enable_if_t<IsConst || !OtherConst>>
  Element_(const Element_<OtherConst>& other) :
      tree(other.tree), index(other.index) {}

  template <
      bool OtherConst,
      typename = std::enable_if_t<IsConst || !OtherConst>>

  Element_(Element_<OtherConst>&& other) :
      tree(other.tree), index(other.index) {}

  Element_() : tree(nullptr), index(-1) {}

  operator bool() const {
    return index != -1;
  }

  friend bool operator==(const Element_& lhs, const Element_& rhs) {
    return lhs.tree == rhs.tree && lhs.index == rhs.index;
  }

private:
  Element_(tree_ptr_t tree, int index) : tree(tree), index(index) {}

  tree_ptr_t tree;
  int index;

  friend class Tree;
};

template <typename T>
Var<T> Tree::variable(const std::function<T()>& construct) {
  assert(parent_ != -1);
  int variable;
  if (variable_current_ == -1) {
    variable = elements[parent_].first_variable;
  } else {
    variable = variables[variable_current_].next;
  }
  if (variable == -1) {
    if (!elements[parent_].is_new) {
      throw WindowError("Changed the number of variable nodes");
    }
    variable = create_variable(parent_);
    variables[variable].data = construct();
  }
  variable_current_ = variable;
  return Variable<T>(this, variable);
}

inline Element Tree::root() {
  return Element(this, root_);
}

inline ConstElement Tree::root() const {
  return ConstElement(this, root_);
}

inline Element Tree::current() {
  return Element(this, current_);
}

inline ConstElement Tree::current() const {
  return ConstElement(this, current_);
}

} // namespace datagui

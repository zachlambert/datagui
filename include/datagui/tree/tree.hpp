#pragma once

#include "datagui/tree/state.hpp"
#include "datagui/tree/unique_any.hpp"
#include "datagui/tree/vector_map.hpp"
#include <assert.h>
#include <functional>
#include <stack>
#include <stdexcept>
#include <string>

namespace datagui {

class DataContainer {
public:
  virtual int emplace() = 0;
  virtual void pop(int index) = 0;
};

template <typename Data>
requires std::is_default_constructible_v<Data>
class DataContainerImpl : public DataContainer {
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
  int type;
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

  ElementNode(int type, int data_index) : type(type), data_index(data_index) {}
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
  class UsageError : public std::runtime_error {
  public:
    UsageError(const std::string& message) : std::runtime_error(message) {}
  };

  Tree() {}

  void begin();
  void end();

  Element next(int type = -1, const std::string& key = "");
  bool down_if();
  void down();
  void up();

  Element root();
  ConstElement root() const;

  template <typename T>
  Var<T> variable(const std::function<T()>& construct = []() { return T(); });

  template <typename Data>
  int create_element_type() {
    data_containers.push_back(std::make_unique<DataContainerImpl<Data>>());
    return data_containers.size() - 1;
  }

private:
  int create_element(int parent, int prev, int type);
  void rerender_element(int node, int type);
  void remove_element(int node);

  int create_variable(int element);
  void remove_variable(int variable);

  void variable_mutate(int variable);
  void set_revisit(int node);
  void set_rerender(int node);

  std::string element_debug(int element) const;

  VectorMap<ElementNode> elements;
  std::vector<std::unique_ptr<DataContainer>> data_containers;
  VectorMap<VariableNode> variables;

  bool is_new = true;
  bool render_in_progress = false;
  int root_ = -1;
  int parent_ = -1;
  int current_ = -1;
  int variable_current_ = -1;
  std::stack<int> variable_stack_;
  std::vector<int> queue_revisit_;
  std::vector<int> queue_rerender_;
  std::vector<int> queue_remove_;

  template <typename T, bool IsConst>
  friend class Variable_;
  template <bool IsConst>
  friend class Element_;
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
  int type() const {
    return tree->elements[index].type;
  }

  Element_ first_child() const {
    assert(index != -1);
    return Element_(tree, tree->elements[index].first_child);
  }
  Element_ last_child() const {
    assert(index != -1);
    return Element_(tree, tree->elements[index].last_child);
  }
  Element_ next() const {
    assert(index != -1);
    return Element_(tree, tree->elements[index].next);
  }
  Element_ prev() const {
    assert(index != -1);
    return Element_(tree, tree->elements[index].prev);
  }
  Element_ parent() const {
    assert(index != -1);
    return Element_(tree, tree->elements[index].parent);
  }

  state_ref_t operator*() const {
    return tree->elements[index].state;
  }
  state_ptr_t operator->() const {
    return &tree->elements[index].state;
  }

  template <typename Data>
  std::conditional_t<IsConst, const Data&, Data&> data() const {
    const auto& element = tree->elements[index];
    auto container = tree->data_containers[int(element.type)];
    auto container_t = dynamic_cast<DataContainerImpl<Data>>(container);
    assert(container_t);
    return container_t->get(element.data_index);
  }

  template <typename Data>
  std::conditional_t<IsConst, const Data&, Data&> data_if() const {
    const auto& element = tree->elements[index];
    auto container = tree->element_containers[int(element.type)];
    auto container_t = dynamic_cast<DataContainerImpl<Data>>(container);
    if (!container_t) {
      return nullptr;
    }
    return &container_t->get(element.data_index);
  }

  bool visible() const {
    return tree->elements[index].visible;
  }
  std::string debug() const {
    return tree->node_debug(index);
  }

  void trigger() const {
    tree->queue_revisit_.emplace_back(index);
  }

  bool rerender() const {
    return tree->elements[index].rerender;
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
  if (parent_ == -1) {
    throw UsageError("Cannot create a variable outside of a container");
  }
  int variable;
  if (variable_current_ == -1) {
    variable = elements[parent_].first_variable;
  } else {
    variable = variables[variable_current_].next;
  }
  if (variable == -1) {
    if (!elements[parent_].is_new) {
      throw UsageError("Changed the number of variable nodes");
    }
    variable = create_variable(parent_);
    variables[variable].data = construct();
  }
  variable_current_ = variable;
  return Variable<T>(this, variable);
}

inline Element Tree::root() {
  if (render_in_progress) {
    throw UsageError(
        "Cannot traverse the tree data while rendering is in progress");
  }
  return Element(this, root_);
}

inline ConstElement Tree::root() const {
  if (render_in_progress) {
    throw UsageError(
        "Cannot traverse the tree data while rendering is in progress");
  }
  return ConstElement(this, root_);
}

} // namespace datagui

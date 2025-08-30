#pragma once

#include "datagui/log.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/types/unique_any.hpp"
#include "datagui/types/vector_map.hpp"
#include <assert.h>
#include <functional>
#include <stack>
#include <stdexcept>
#include <string>

namespace datagui {

struct ElementNode {
  Element element;

  int id;
  bool revisit = true;

  int parent = -1;
  int prev = -1;
  int next = -1;
  int first_child = -1;
  int last_child = -1;
  int first_variable = -1;

  int version = 0; // Debug information
};

struct VariableNode {
  int element;

  UniqueAny data;
  UniqueAny data_new;
  bool modified = false;

  // Linked list of data nodes for a given gui node
  int prev = -1;
  int next = -1;

  VariableNode(int element) : element(element) {}
};

// Forward declare Element_

template <bool IsConst>
class ElementPtr_;
using ElementPtr = ElementPtr_<false>;
using ConstElementPtr = ElementPtr_<true>;

// Forward declare Var_

template <typename T, bool IsConst>
class Var_;
template <typename T>
using Var = Var_<T, false>;
template <typename T>
using ConstVar = Var_<T, true>;

class Tree {
  template <typename T, bool IsConst>
  friend class Var_;

public:
  class UsageError : public std::runtime_error {
  public:
    UsageError(const std::string& message) : std::runtime_error(message) {}
  };

  Tree() {}

  bool begin();
  void end();

  ElementPtr next(int id = -1);
  bool down_if();
  void down();
  void up();

  ElementPtr root();
  ConstElementPtr root() const;

  template <typename T>
  Var<T> variable(const std::function<T()>& construct = []() { return T(); });

  int get_id() {
    return next_id++;
  }

private:
  int create_element(int parent, int prev, int id);
  void remove_element(int node);

  int create_variable(int element);
  void remove_variable(int variable);

  void set_revisit(int node);

  std::string element_debug(int element) const;

  VectorMap<ElementNode> elements;
  VectorMap<VariableNode> variables;

  bool active_ = false;
  int root_ = -1;
  int parent_ = -1;
  int current_ = -1;
  int variable_current_ = -1;

  int next_id = 0;

  int depth = 0; // Debugging

  std::stack<int> variable_stack_;
  std::vector<int> queue_revisit_;
  std::vector<int> queue_modified_variables_;
  std::vector<int> modified_variables_;

  int external_first_variable_ = -1;

  template <typename T, bool IsConst>
  friend class Var_;
  template <bool IsConst>
  friend class ElementPtr_;
};

template <typename T, bool IsConst>
class Var_ {
  using data_ptr_t = std::conditional_t<IsConst, const T*, T*>;
  using data_ref_t = std::conditional_t<IsConst, const T&, T&>;

public:
  data_ref_t operator*() const {
    auto data_ptr = tree->variables[variable].data.cast<T>();
    return *data_ptr;
  }
  data_ptr_t operator->() const {
    auto data_ptr = tree->variables[variable].data.cast<T>();
    return data_ptr;
  }
  void set(const T& value) const {
    static_assert(!IsConst);
    assert(tree->variables[variable].data.cast<T>());
    tree->variables[variable].data_new = UniqueAny::Make<T>(value);
    tree->queue_modified_variables_.push_back(variable);
  }
  void set(T&& value) const {
    static_assert(!IsConst);
    assert(tree->variables[variable].data.cast<T>());
    tree->variables[variable].data_new = UniqueAny::Make<T>(std::move(value));
    tree->queue_modified_variables_.push_back(variable);
  }
  bool modified() const {
    return tree->variables[variable].modified;
  }
  bool has_value() const {
    return tree->variables[variable].data;
  }

  template <
      bool OtherConst,
      typename = std::enable_if_t<IsConst || !OtherConst>>
  Var_(const Var_<T, OtherConst>& other) :
      tree(other.tree), variable(other.variable), data_ptr(other.data_ptr) {}

  template <
      bool OtherConst,
      typename = std::enable_if_t<IsConst || !OtherConst>>
  Var_(Var_<T, OtherConst>&& other) :
      tree(other.tree), variable(other.variable), data_ptr(other.data_ptr) {}

  Var_() : tree(nullptr), variable(-1), data_ptr(nullptr) {}

  operator bool() const {
    return tree;
  }

private:
  Var_(Tree* tree, int variable) :
      tree(tree),
      variable(variable),
      data_ptr(tree->variables[variable].data.cast<T>()) {
    assert(data_ptr);
  }

  Tree* tree;
  int variable;
  data_ptr_t data_ptr;

  friend class Tree;
};
template <typename T>
using Variable = Var_<T, false>;
template <typename T>
using ConstVariable = Var_<T, true>;

template <bool IsConst>
class ElementPtr_ {
  using tree_ptr_t = std::conditional_t<IsConst, const Tree*, Tree*>;
  using ptr_t = std::conditional_t<IsConst, const Element*, Element*>;
  using ref_t = std::conditional_t<IsConst, const Element&, Element&>;

public:
  int type() const {
    return tree->elements[index].type;
  }

  ElementPtr_ first_child() const {
    assert(index != -1);
    return ElementPtr_(tree, tree->elements[index].first_child);
  }
  ElementPtr_ last_child() const {
    assert(index != -1);
    return ElementPtr_(tree, tree->elements[index].last_child);
  }
  ElementPtr_ next() const {
    assert(index != -1);
    return ElementPtr_(tree, tree->elements[index].next);
  }
  ElementPtr_ prev() const {
    assert(index != -1);
    return ElementPtr_(tree, tree->elements[index].prev);
  }
  ElementPtr_ parent() const {
    assert(index != -1);
    return ElementPtr_(tree, tree->elements[index].parent);
  }

  ref_t operator*() const {
    return tree->elements[index].element;
  }
  ptr_t operator->() const {
    return &tree->elements[index].element;
  }
  ptr_t get() const {
    return &tree->elements[index].element;
  }

  template <typename T>
  std::conditional_t<IsConst, const T&, T&> cast() const {
    static_assert(std::is_base_of_v<Element, T>);
    using T_ptr = std::conditional_t<IsConst, const T*, T*>;
    return *dynamic_cast<T_ptr>(tree->elements[index].element.get());
  }

  template <typename T>
  std::conditional_t<IsConst, const T*, T*> cast_if() const {
    static_assert(std::is_base_of_v<Element, T>);
    using T_ptr = std::conditional_t<IsConst, const T*, T*>;
    return dynamic_cast<T_ptr>(tree->elements[index].element.get());
  }

  std::string debug() const {
    return tree->element_debug(index);
  }

  void revisit(bool revisit = true) const {
    if (!revisit) {
      return;
    }
    DATAGUI_LOG("ElementPtr_::revisit", "REVISIT: element=%i", index);
    tree->queue_revisit_.emplace_back(index);
  }

  template <
      bool OtherConst,
      typename = std::enable_if_t<IsConst || !OtherConst>>
  ElementPtr_(const ElementPtr_<OtherConst>& other) :
      tree(other.tree), index(other.index) {}

  template <
      bool OtherConst,
      typename = std::enable_if_t<IsConst || !OtherConst>>

  ElementPtr_(ElementPtr_<OtherConst>&& other) :
      tree(other.tree), index(other.index) {}

  ElementPtr_() : tree(nullptr), index(-1) {}

  operator bool() const {
    return index != -1;
  }

  friend bool operator==(const ElementPtr_& lhs, const ElementPtr_& rhs) {
    return lhs.tree == rhs.tree && lhs.index == rhs.index;
  }

private:
  ElementPtr_(tree_ptr_t tree, int index) : tree(tree), index(index) {}

  tree_ptr_t tree;
  int index;

  friend class Tree;

  template <bool OtherConst>
  friend class ElementPtr_;
};

template <typename T>
Var<T> Tree::variable(const std::function<T()>& construct) {
  int variable;

  if (parent_ == -1) {
    // Variables are "external" - not within any container element
    if (variable_current_ == -1) {
      variable = external_first_variable_;
    } else {
      variable = variables[external_first_variable_].next;
    }
  } else {
    // Variables are within a container element
    if (variable_current_ == -1) {
      variable = elements[parent_].first_variable;
    } else {
      variable = variables[variable_current_].next;
    }
  }

  if (variable == -1) {
    if (parent_ == -1) {
      variable = create_variable(-1);
      variables[variable].data = UniqueAny::Make<T>(construct());
      DATAGUI_LOG("Tree::variable", "Created external variable: %i", variable);
    } else {
      variable = create_variable(parent_);
      variables[variable].data = UniqueAny::Make<T>(construct());
      DATAGUI_LOG("Tree::variable", "Created internal variable: %i", variable);
    }
  }
  variable_current_ = variable;
  return Variable<T>(this, variable);
}

inline ElementPtr Tree::root() {
  return ElementPtr(this, root_);
}

inline ConstElementPtr Tree::root() const {
  return ConstElementPtr(this, root_);
}

} // namespace datagui

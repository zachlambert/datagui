#pragma once

#include "datagui/log.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/types/unique_any.hpp"
#include "datagui/types/vector_map.hpp"
#include <assert.h>
#include <chrono>
#include <functional>
#include <stack>
#include <stdexcept>
#include <string>
#include <variant>

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
  int first_dependency = -1;
};

struct VarNode {
  int element;

  UniqueAny data;
  int version = 0;

  // Linked list of data nodes for a given gui node
  int prev = -1;
  int next = -1;

  VarNode(int element) : element(element) {}
};

struct DependencyVar {
  int variable;
  int version;
  DependencyVar(int variable, int version) :
      variable(variable), version(version) {}
};

struct DependencyCondition {
  std::function<bool()> condition;
  DependencyCondition(const std::function<bool()>& condition) :
      condition(condition) {}
};

struct DependencyTimeout {
  using clock_t = std::chrono::high_resolution_clock;
  clock_t::time_point timepoint;
  DependencyTimeout(const clock_t::time_point& timepoint) :
      timepoint(timepoint) {}
};

using Dependency =
    std::variant<DependencyVar, DependencyCondition, DependencyTimeout>;

struct DependencyNode {
  int element;
  Dependency dependency;

  int next = -1;
  int prev = -1;

  DependencyNode(int element, const Dependency& dependency) :
      element(element), dependency(dependency) {}
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

  using clock_t = std::chrono::high_resolution_clock;

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
  Var<T> variable(const T& initial_value = T());

  template <typename T, bool Const>
  void on_variable(const Var_<T, Const>& var);
  void on_condition(const std::function<bool()>& condition) {
    if (parent_ == -1) {
      throw UsageError("Cannot add a dependency at the root");
    }
    create_dependency(parent_, DependencyCondition(condition));
  }
  void on_timeout(double period) {
    if (parent_ == -1) {
      throw UsageError("Cannot add a dependency at the root");
    }
    auto now = clock_t::now();
    auto future = now + std::chrono::nanoseconds(std::int64_t(period * 1e9));
    create_dependency(parent_, DependencyTimeout(future));
  }

  int get_id() {
    return next_id++;
  }

private:
  int create_element(int parent, int prev, int id);
  void remove_element(int node);

  int get_variable(UniqueAny&& value);
  int create_variable(int element);
  void clear_variables(int element);

  void create_dependency(int element, const Dependency& value);
  void clear_dependencies(int element);

  void set_revisit(int node);

  std::string element_debug(int element) const;

  VectorMap<ElementNode> elements;
  VectorMap<VarNode> variables;
  VectorMap<DependencyNode> dependencies;

  bool active_ = false;
  int root_ = -1;
  int parent_ = -1;
  int current_ = -1;
  int variable_current_ = -1;

  int next_id = 0;

  int depth = 0; // Debugging

  std::stack<int> variable_stack_;
#if 0
  std::vector<int> queue_revisit_;
  std::vector<std::pair<int, bool>>
      queue_modified_variables_; // [variable, internally modified?]
#endif

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
  void set_modified(bool modified = true) const {
    if (modified) {
      tree->variables[variable].version++;
    }
  }
  void set(const T& value) const {
    *tree->variables[variable].data.cast<T>() = value;
    tree->variables[variable].version++;
  }
#if 0
  void set(const T& value) const {
    static_assert(!IsConst);
    assert(tree->variables[variable].data.cast<T>());
    tree->variables[variable].data_new = UniqueAny::Make<T>(value);
    tree->queue_modified_variables_.emplace_back(variable, false);
  }
  void set(T&& value) const {
    static_assert(!IsConst);
    assert(tree->variables[variable].data.cast<T>());
    tree->variables[variable].data_new = UniqueAny::Make<T>(std::move(value));
    tree->queue_modified_variables_.emplace_back(variable, false);
  }
  bool modified() const {
    return tree->variables[variable].modified;
  }
  bool has_value() const {
    return tree->variables[variable].data;
  }
#endif

  template <
      bool OtherConst,
      typename = typename std::enable_if_t<IsConst || !OtherConst>>
  Var_(const Var_<T, OtherConst>& other) :
      tree(other.tree), variable(other.variable), data_ptr(other.data_ptr) {}

  template <
      bool OtherConst,
      typename = typename std::enable_if_t<IsConst || !OtherConst>>
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

  int version() const {
    return tree->variables[variable].version;
  }

  Tree* tree;
  int variable;
  data_ptr_t data_ptr;

  friend class Tree;

  template <typename T_, bool Const_>
  friend class Var_;

  friend class GuiWriter;
  friend class GuiReader;
  friend class Gui;
#if 0
  // For the following methods

  void mutate(const T& value) const {
    tree->variables[variable].data = UniqueAny::Make<T>(value);
  }
  void set_internal(const T& value) const {
    static_assert(!IsConst);
    assert(tree->variables[variable].data.cast<T>());
    tree->variables[variable].data_new = UniqueAny::Make<T>(value);
    tree->queue_modified_variables_.emplace_back(variable, true);
  }
  void set_internal(T&& value) const {
    static_assert(!IsConst);
    assert(tree->variables[variable].data.cast<T>());
    tree->variables[variable].data_new = UniqueAny::Make<T>(std::move(value));
    tree->queue_modified_variables_.emplace_back(variable, true);
  }
  bool modified_external() const {
    return modified() && !tree->variables[variable].modified_internal;
  }
#endif
};

#if 0
template <typename T>
using Var = Var_<T, false>;
template <typename T>
using ConstVar = Var_<T, true>;
#endif

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
    tree->set_revisit(index);
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
Var<T> Tree::variable(const T& initial_value) {
  return Var<T>(this, get_variable(UniqueAny::Make<T>(initial_value)));
}

template <typename T, bool Const>
void Tree::on_variable(const Var_<T, Const>& var) {
  if (var.tree != this) {
    throw UsageError("Cannot depend on a variable from another tree");
  }
  if (parent_ == -1) {
    throw UsageError("Cannot add a dependency at the root");
  }
  create_dependency(
      parent_,
      DependencyVar(var.variable, variables[var.variable].version));
}

inline ElementPtr Tree::root() {
  return ElementPtr(this, root_);
}

inline ConstElementPtr Tree::root() const {
  return ConstElementPtr(this, root_);
}

} // namespace datagui

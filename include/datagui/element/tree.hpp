#pragma once

#include "datagui/element/element.hpp"
#include "datagui/element/state.hpp"
#include "datagui/element/unique_any.hpp"
#include "datagui/element/vector_map.hpp"
#include <assert.h>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <variant>

namespace datagui {

class ElementError : public std::runtime_error {
public:
  ElementError(const std::string& message) : std::runtime_error(message) {}
};

class Tree {
  using clock_t = std::chrono::high_resolution_clock;

  // ===========================================================
  // ElementNode

  struct ElementNode {
    std::size_t id;
    bool dirty = true;

    int parent = -1;
    int prev = -1;
    int next = -1;
    int first_child = -1;
    int last_child = -1;
    int first_variable = -1;
    int first_dependency = -1;

    State state;
    Type type;
    std::size_t type_index;
  };

  // ===========================================================
  // VarNode

  struct VarNode {
    int element;

    UniqueAny data;
    int version = 0;

    // Linked list of data nodes for a given gui node
    int prev = -1;
    int next = -1;

    VarNode(int element) : element(element) {}
  };

public:
  // ===========================================================
  // DependencyNode

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

  // ===========================================================
  // Var

  template <typename T, bool IsConst>
  class Var_ {
    using data_ptr_t = std::conditional_t<IsConst, const T*, T*>;
    using data_ref_t = std::conditional_t<IsConst, const T&, T&>;

  public:
    const T& operator*() const {
      assert(tree && variable != -1);
      auto data_ptr = tree->variables[variable].data.cast<T>();
      return *data_ptr;
    }
    const T* operator->() const {
      assert(tree && variable != -1);
      auto data_ptr = tree->variables[variable].data.cast<T>();
      return data_ptr;
    }
    void set(const T& value) const {
      assert(tree && variable != -1);
      static_assert(!IsConst);
      *tree->variables[variable].data.cast<T>() = value;
      tree->variables[variable].version++;
    }
    T& mut() const {
      assert(tree && variable != -1);
      static_assert(!IsConst);
      tree->variables[variable].version++;
      return *tree->variables[variable].data.cast<T>();
    }

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
  };

  template <bool Const>
  class ElementPtr_;

  template <bool Const>
  class VarPtr_ {
  public:
    VarPtr_ next() {
      return VarPtr_(tree, element, tree->variables[variable].next);
    }
    operator bool() const {
      return variable != -1;
    }
    bool valid() const {
      return tree;
    }

    template <typename T>
    Var_<T, Const> create(const T& value) {
      assert(tree && variable == -1);
      variable = tree->create_variable(element);
      tree->variables[variable].data = UniqueAny::Make<T>(value);
      tree->variables[variable].version = 0;
      return Var_<T, Const>(tree, variable);
    }

    template <typename T>
    Var_<T, Const> as() {
      assert(tree && variable != -1);
      return Var_<T, Const>(tree, variable);
    }

    VarPtr_() : tree(nullptr), element(-1), variable(-1) {}

  private:
    VarPtr_(Tree* tree, int element, int variable) :
        tree(tree), element(element), variable(variable) {}

    Tree* tree;
    int element;
    int variable;

    template <bool Const_>
    friend class ElementPtr_;
    friend class Tree;
  };

  using VarPtr = VarPtr_<false>;
  using ConstVarPtr = VarPtr_<true>;

  template <typename T>
  using Var = Var_<T, false>;
  template <typename T>
  using ConstVar = Var_<T, true>;

  // ===========================================================
  // ElementPtr

  template <bool IsConst>
  class ElementPtr_ {
  public:
    Type type() const {
      assert(tree && index != -1);
      return tree->elements[index].type;
    }

#define PROPS_METHOD(T, name) \
  std::conditional_t<IsConst, const T&, T&> name() const { \
    assert(tree&& index != -1); \
    assert(tree->elements.contains(index)); \
    const auto& element = tree->elements[index]; \
    assert(element.type == Type::T); \
    assert(tree->name.contains(element.type_index)); \
    return tree->name[element.type_index]; \
  }

    PROPS_METHOD(Button, button)
    PROPS_METHOD(Checkbox, checkbox)
    PROPS_METHOD(Dropdown, dropdown)
    PROPS_METHOD(Floating, floating)
    PROPS_METHOD(Labelled, labelled)
    PROPS_METHOD(Section, section)
    PROPS_METHOD(Series, series)
    PROPS_METHOD(TextBox, text_box)
    PROPS_METHOD(TextInput, text_input)

#undef PROPS_METHOD

    ElementPtr_ parent() const {
      assert(tree && index != -1);
      if (parent_ == -1) {
        return ElementPtr_(tree, -1, -1);
      } else {
        return ElementPtr_(tree, tree->elements[parent_].parent, parent_);
      }
    }

    ElementPtr_ child() const {
      assert(tree && index != -1);
      return ElementPtr_(tree, index, tree->elements[index].first_child);
    }
    ElementPtr_ last_child() const {
      assert(tree && index != -1);
      return ElementPtr_(tree, index, tree->elements[index].last_child);
    }
    void clear_children() const {
      assert(tree && index != -1);
      tree->remove_element(index, true); // Children only
    }

    ElementPtr_ next() const {
      assert(tree && index != -1);
      assert(parent_ == tree->elements[index].parent);
      return ElementPtr_(tree, parent_, tree->elements[index].next);
    }
    ElementPtr_ prev() const {
      assert(tree && index != -1);
      assert(parent_ == tree->elements[index].parent);
      return ElementPtr_(tree, parent_, tree->elements[index].prev);
    }

    std::conditional_t<IsConst, const State&, State&> state() const {
      assert(tree && index != -1);
      return tree->elements[index].state;
    }

    operator bool() const {
      return index != -1;
    }

    void create(Type type, std::size_t id = 0) {
      assert(tree);
      if (parent_ == -1) {
        index = tree->create_element(parent_, -1, id, type);
      } else {
        assert(tree->elements.contains(parent_));
        int prev = tree->elements[parent_].last_child;
        index = tree->create_element(parent_, prev, id, type);
      }
    }
    void reset(Type type, std::size_t id = 0) const {
      assert(tree && index != -1);
      tree->reset_element(index, id, type);
    }
    ElementPtr_ erase() const {
      assert(tree && index != -1);
      int next = tree->elements[index].next;
      tree->remove_element(index);
      return ElementPtr_(tree, parent_, next);
    }

    bool expect(Type type, std::size_t id = -1) {
      assert(tree);
      while (index != -1 && id != tree->elements[index].id) {
        (*this) = erase();
      }
      if (index == -1) {
        create(type, id);
        return true;
      }
      if (type != tree->elements[index].type) {
        throw ElementError("Incorrect type");
      }
      return false;
    }
    void expect_end() {
      assert(tree);
      while (index != -1) {
        (*this) = erase();
      }
    }

    friend bool operator==(const ElementPtr_& lhs, const ElementPtr_& rhs) {
      return lhs.index == rhs.index;
    }

    int id() const {
      assert(tree && index != -1);
      return tree->elements[index].id;
    }

    bool dirty() const {
      assert(tree && index != -1);
      return tree->elements[index].dirty;
    }
    void set_dirty(bool dirty = true) {
      assert(tree && index != -1);
      if (dirty) {
        tree->set_dirty(index);
      }
    }

    void clear_vars() const {
      assert(tree && index != -1);
      tree->clear_vars(index);
    }
    VarPtr var() const {
      assert(tree && index != -1);
      return VarPtr(tree, index, tree->elements[index].first_variable);
    }
    ConstVarPtr const_var() const {
      assert(tree && index != -1);
      return ConstVarPtr(tree, index, tree->elements[index].first_variable);
    }

    void clear_dependencies() const {
      assert(tree && index != -1);
      tree->clear_dependencies(index);
    }

    template <typename T, bool Const>
    void add_variable_dep(const Var_<T, Const>& var) const {
      tree->create_dependency(
          index,
          DependencyVar(var.variable, var.version()));
    }
    void add_condition_dependency(const std::function<bool()>& condition) {
      tree->create_dependency(index, DependencyCondition(condition));
    }
    void add_timeout_dependency(double period) {
      auto now = clock_t::now();
      auto future = now + std::chrono::nanoseconds(std::int64_t(period * 1e9));
      tree->create_dependency(index, DependencyTimeout(future));
    }

    template <
        bool OtherConst,
        typename = std::enable_if_t<IsConst || !OtherConst>>
    ElementPtr_(const ElementPtr_<OtherConst>& other) :
        tree(other.tree), parent_(other.parent_), index(other.index) {}

    template <
        bool OtherConst,
        typename = std::enable_if_t<IsConst || !OtherConst>>
    ElementPtr_(ElementPtr_<OtherConst>&& other) :
        tree(other.tree), parent_(other.parent_), index(other.index) {}

    ElementPtr_() : tree(nullptr), parent_(-1), index(-1) {}

  private:
    using tree_ptr_t = std::conditional_t<IsConst, const Tree*, Tree*>;
    ElementPtr_(tree_ptr_t tree, int parent, int index) :
        tree(tree), parent_(parent), index(index) {}

    tree_ptr_t tree;
    int parent_;
    int index;

    friend class Tree;

    template <bool OtherConst>
    friend class ElementPtr_;
  };
  using ElementPtr = ElementPtr_<false>;
  using ConstElementPtr = ElementPtr_<true>;

  Tree() {}

  void poll();
  void clear_dirty();

  ElementPtr root() {
    return ElementPtr(this, -1, root_);
  }

  ConstElementPtr root() const {
    return ConstElementPtr(this, -1, root_);
  }

  VarPtr var() {
    return VarPtr(this, -1, external_var_);
  }

private:
  int create_element(int parent, int prev, int id, Type type);
  void reset_element(int element, int id, Type type);
  void remove_element(int element, bool children_only = false);

  int emplace_type(Type type);
  void pop_type(Type type, std::size_t index);

  int create_variable(int element);
  void clear_variables(int element);

  void create_dependency(int element, const Dependency& value);
  void clear_dependencies(int element);

  void set_dirty(int node);

  int root_ = -1;
  VectorMap<ElementNode> elements;
  VectorMap<VarNode> variables;
  VectorMap<DependencyNode> dependencies;
  int external_var_ = -1;

  VectorMap<Button> button;
  VectorMap<Checkbox> checkbox;
  VectorMap<Dropdown> dropdown;
  VectorMap<Floating> floating;
  VectorMap<Labelled> labelled;
  VectorMap<Section> section;
  VectorMap<Series> series;
  VectorMap<TextBox> text_box;
  VectorMap<TextInput> text_input;
};

using ElementPtr = Tree::ElementPtr;
using ConstElementPtr = Tree::ConstElementPtr;

using VarPtr = Tree::VarPtr;
using ConstVarPtr = Tree::ConstVarPtr;

template <typename T>
using Var = Tree::Var<T>;
template <typename T>
using ConstVar = Tree::ConstVar<T>;

} // namespace datagui

#pragma once

#include "datagui/log.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/tree/props.hpp"
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

class Tree {
  using clock_t = std::chrono::high_resolution_clock;

  // ===========================================================
  // ElementNode

  struct ElementNode {
    int id;
    bool revisit = true;

    int parent = -1;
    int prev = -1;
    int next = -1;
    int first_child = -1;
    int last_child = -1;
    int first_variable = -1;
    int first_dependency = -1;

    Element element;
    PropsType props_type;
    std::size_t props_index;
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
      auto data_ptr = tree->variables[variable].data.cast<T>();
      return *data_ptr;
    }
    const T* operator->() const {
      auto data_ptr = tree->variables[variable].data.cast<T>();
      return data_ptr;
    }
    void set(const T& value) const {
      *tree->variables[variable].data.cast<T>() = value;
      tree->variables[variable].version++;
    }
    T& mut() const {
      static_assert(!IsConst);
      tree->variables[variable].version++;
      if (tree->active_) {
        throw Tree::UsageError(
            "Cannot mutate a variable while building the tree");
      }
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
  };

  template <typename T>
  using Var = Var_<T, false>;
  template <typename T>
  using ConstVar = Var_<T, true>;

  // ===========================================================
  // ElementPtr

#define PROPS_METHOD(Props, props) \
  std::conditional_t<IsConst, const Props&, Props&> props() const { \
    const auto& element = tree->elements[index]; \
    assert(Props::TYPE == element.type); \
    return tree->props[element.props_index]; \
  }

  template <bool IsConst>
  class ElementPtr_ {
    using tree_ptr_t = std::conditional_t<IsConst, const Tree*, Tree*>;
    using ptr_t = std::conditional_t<IsConst, const Element*, Element*>;
    using ref_t = std::conditional_t<IsConst, const Element&, Element&>;

  public:
    PropsType props_type() const {
      return tree->elements[index].type;
    }

    PROPS_METHOD(ButtonProps, button_props)
    PROPS_METHOD(CheckboxProps, checkbox_props)
    PROPS_METHOD(DropdownProps, dropdown_props)
    PROPS_METHOD(FloatingProps, floating_props)
    PROPS_METHOD(LabelledProps, labelled_props)
    PROPS_METHOD(SectionProps, section_props)
    PROPS_METHOD(SeriesProps, series_props)
    PROPS_METHOD(TextBoxProps, text_box_props)
    PROPS_METHOD(TextInputProps, text_input_props)

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
      return index > 0 && tree->elements.contains(index);
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
  using ElementPtr = ElementPtr_<false>;
  using ConstElementPtr = ElementPtr_<true>;

#undef PROPS_METHOD

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

  ElementPtr current() {
    return ElementPtr(this, current_);
  }

  ConstElementPtr current() const {
    return ConstElementPtr(this, current_);
  }

  ElementPtr root() {
    return ElementPtr(this, root_);
  }

  ConstElementPtr root() const {
    return ConstElementPtr(this, root_);
  }

  template <typename T>
  Var<T> variable(const T& initial_value) {
    auto [variable, is_new] = get_variable();
    if (is_new) {
      variables[variable].data = UniqueAny::Make<T>(initial_value);
    }
    return Var<T>(this, variable);
  }

  template <typename T>
  Var<T> variable(const std::function<T()>& create_initial_value) {
    auto [variable, is_new] = get_variable();
    if (is_new) {
      variables[variable].data = UniqueAny::Make<T>(create_initial_value());
    }
    return Var<T>(this, variable);
  }

  template <typename T, bool Const>
  void on_variable(const Var_<T, Const>& var) {
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
  int create_element(int parent, int prev, int id, ElementType type);
  void remove_element(int node);

  std::tuple<int, bool> get_variable();
  int create_variable(int element);
  void clear_variables(int element);

  void create_dependency(int element, const Dependency& value);
  void clear_dependencies(int element);

  void set_revisit(int node);

  std::string element_debug(int element) const;

  VectorMap<ElementNode> elements;
  VectorMap<VarNode> variables;
  VectorMap<DependencyNode> dependencies;

  VectorMap<ButtonProps> button_props;
  VectorMap<CheckboxProps> checkbox_props;
  VectorMap<DropdownProps> dropdown_props;
  VectorMap<FloatingProps> floating_props;
  VectorMap<LabelledProps> labelled_props;
  VectorMap<SectionProps> section_props;
  VectorMap<SeriesProps> series_props;
  VectorMap<TextBoxProps> text_box_props;
  VectorMap<TextInputProps> text_input_props;

  bool active_ = false;
  int root_ = -1;
  int parent_ = -1;
  int current_ = -1;
  int variable_current_ = -1;

  int next_id = 0;

  std::stack<int> variable_stack_;

  int external_first_variable_ = -1;

  template <typename T, bool IsConst>
  friend class Var_;
  template <bool IsConst>
  friend class ElementPtr_;
};

} // namespace datagui

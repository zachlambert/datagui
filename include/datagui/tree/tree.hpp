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

enum class NodeType { External, Primitive, Container, Optional, Variant };

struct Node {
  NodeType type = NodeType::Primitive;
  bool open = false;
  std::string open_label = "";
  std::string label = "";

  int parent = -1;
  int prev = -1;
  int next = -1;
  int first_child = -1;
  int last_child = -1;

  bool is_new = true;
  bool triggered = true;
  bool visible = true;

  int first_variable = -1;
  int first_dep = -1;

  int version = 0; // Debug information
  State state;
};

struct VariableNode {
  int node;

  UniqueAny data;
  bool modified;

  // Linked list of data nodes for a given gui node
  int prev;
  int next;
  int first_dep;

  VariableNode(int node) :
      node(node), modified(false), prev(-1), next(-1), first_dep(-1) {}
};

struct DepNode {
  int node;

  // Linked list of dependencies for a given variable node
  int variable_node;
  int prev;
  int next;

  // Linked list of dependencies for a given gui node
  int node_prev;
  int node_next;

  DepNode(int node, int variable_node) :
      node(node),
      variable_node(variable_node),
      prev(-1),
      next(-1),
      node_prev(-1),
      node_next(-1) {}
};

class Tree {
public:
  template <typename T, bool IsConst>
  class Variable_ {
    using data_ptr_t = std::conditional_t<IsConst, const T*, T*>;

  public:
    const T& operator*() const {
      tree->variable_access(variable_node);
      return *data_ptr;
    }
    const T* operator->() const {
      tree->variable_access(variable_node);
      return data_ptr;
    }
    T& mut() const {
      static_assert(!IsConst);
      tree->variable_mutate(variable_node);
      return *data_ptr;
    }
    // TEMP
    const T& immut() const {
      return *data_ptr;
    }
    bool modified() const {
      tree->variable_access(variable_node);
      return tree->variable_nodes[variable_node].modified;
    }

    template <
        bool OtherConst,
        typename = std::enable_if_t<IsConst || !OtherConst>>
    Variable_(const Variable_<T, OtherConst>& other) :
        tree(other.tree),
        variable_node(other.variable_node),
        data_ptr(other.data_ptr) {}

    template <
        bool OtherConst,
        typename = std::enable_if_t<IsConst || !OtherConst>>
    Variable_(Variable_<T, OtherConst>&& other) :
        tree(other.tree),
        variable_node(other.variable_node),
        data_ptr(other.data_ptr) {}

  private:
    Variable_(Tree* tree, int variable_node) :
        tree(tree),
        variable_node(variable_node),
        data_ptr(tree->variable_nodes[variable_node].data.cast<T>()) {}

    Tree* tree;
    int variable_node;
    data_ptr_t data_ptr;

    friend class Tree;
  };
  template <typename T>
  using Variable = Variable_<T, false>;
  template <typename T>
  using ConstVariable = Variable_<T, true>;

  template <bool IsConst>
  class Ptr_ {
    using tree_ptr_t = std::conditional_t<IsConst, const Tree*, Tree*>;
    using state_ptr_t = std::conditional_t<IsConst, const State*, State*>;
    using state_ref_t = std::conditional_t<IsConst, const State&, State&>;

  public:
    Ptr_ first_child() const {
      assert(index != -1);
      return Ptr_(tree, tree->nodes[index].first_child);
    }
    Ptr_ last_child() const {
      assert(index != -1);
      return Ptr_(tree, tree->nodes[index].last_child);
    }
    Ptr_ next() const {
      assert(index != -1);
      return Ptr_(tree, tree->nodes[index].next);
    }
    Ptr_ prev() const {
      assert(index != -1);
      return Ptr_(tree, tree->nodes[index].prev);
    }
    Ptr_ parent() const {
      assert(index != -1);
      return Ptr_(tree, tree->nodes[index].parent);
    }

    state_ref_t operator*() const {
      return tree->nodes[index].state;
    }
    state_ptr_t operator->() const {
      return &tree->nodes[index].state;
    }
    bool visible() const {
      return tree->nodes[index].visible;
    }
    std::string debug() const {
      return tree->node_debug(index);
    }

    void trigger() const {
      tree->queue_triggered.emplace_back(index, false);
    }

    template <
        bool OtherConst,
        typename = std::enable_if_t<IsConst || !OtherConst>>
    Ptr_(const Ptr_<OtherConst>& other) :
        tree(other.tree), index(other.index) {}

    template <
        bool OtherConst,
        typename = std::enable_if_t<IsConst || !OtherConst>>

    Ptr_(Ptr_<OtherConst>&& other) : tree(other.tree), index(other.index) {}

    Ptr_() : tree(nullptr), index(-1) {}

    operator bool() const {
      return index != -1;
    }

    friend bool operator==(const Ptr_& lhs, const Ptr_& rhs) {
      return lhs.tree == rhs.tree && lhs.index == rhs.index;
    }

  private:
    Ptr_(tree_ptr_t tree, int index) : tree(tree), index(index) {}

    tree_ptr_t tree;
    int index;

    friend class Tree;
  };
  using Ptr = Ptr_<false>;
  using ConstPtr = Ptr_<true>;

  using init_state_t = std::function<void(State& state)>;
  using deinit_node_t = std::function<void(ConstPtr)>;

  Tree(const deinit_node_t& deinit_node);

  void begin();
  void end();

  void container_next(const init_state_t& init_state);
  bool container_down();
  bool optional_down(
      bool open,
      const init_state_t& init_state,
      bool retain = true);
  bool variant_down(
      const std::string& label,
      const init_state_t& init_state,
      bool retain = true);

  void up();

  template <typename T>
  Variable<T> variable(const std::function<T()>& construct = []() {
    return T();
  }) {
    assert(parent_ != -1);
    int variable_node;
    if (parent_variable_current_ == -1) {
      variable_node = nodes[parent_].first_variable;
    } else {
      variable_node = variable_nodes[parent_variable_current_].next;
    }
    if (variable_node == -1) {
      if (!nodes[parent_].is_new) {
        throw WindowError("Changed the number of variable nodes");
      }
      variable_node = create_variable_node(parent_);
      variable_nodes[variable_node].data = construct();
    }
    parent_variable_current_ = variable_node;
    return Variable<T>(this, parent_variable_current_);
  }

  Ptr root() {
    return Ptr(this, root_);
  }
  ConstPtr root() const {
    return ConstPtr(this, root_);
  }

  Ptr current() {
    return Ptr(this, current_);
  }
  ConstPtr current() const {
    return ConstPtr(this, current_);
  }

private:
  struct NodeTrigger {
    const int node;
    const bool immutable;
    NodeTrigger(int node, bool immutable) : node(node), immutable(immutable) {}
  };

  int create_node(int parent, int prev);
  void init_node(int node, const init_state_t& init_state);
  void reset_node(int node);
  void remove_node(int node);

  void set_triggered(int node);

  int create_variable_node(int node);
  void variable_mutate(int data_node);
  void variable_access(int data_node);

  void remove_variable_node(int data_node);
  void remove_dep_node(int dep_node);
  void remove_node_variable_nodes(int node);
  void remove_node_dep_nodes(int node);

  std::string node_debug(int node) const;

  deinit_node_t deinit_node;
  VectorMap<Node> nodes;
  VectorMap<VariableNode> variable_nodes;
  VectorMap<DepNode> dep_nodes;

  bool is_new = true;
  int external_ = -1;
  int root_ = -1;
  int parent_ = -1;
  int current_ = -1;
  int parent_variable_current_ = -1;
  int variable_access_node_ = -1;
  std::stack<int> parent_variable_current_stack_;
  std::vector<NodeTrigger> queue_triggered;
};

} // namespace datagui

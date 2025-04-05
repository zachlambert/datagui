#pragma once

#include "datagui/exception.hpp"
#include "datagui/tree/state.hpp"
#include "datagui/tree/vector_map.hpp"
#include <any>
#include <assert.h>
#include <functional>
#include <stack>
#include <string>

namespace datagui {

enum class NodeType { Primitive, Container, Optional, Variant };

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
  bool needs_visit = true;
  bool visible = true;

  int first_data = -1;
  int first_dep = -1;

  State state;
};

struct DataNode {
  int node;

  std::any data;
  bool modified;

  // Linked list of data nodes for a given gui node
  int prev;
  int next;
  int first_dep;

  DataNode(int node) :
      node(node), modified(false), prev(-1), next(-1), first_dep(-1) {}
};

struct DepNode {
  int node;

  // Linked list of dependencies for a given data node
  int data_node;
  int prev;
  int next;

  // Linked list of dependencies for a given gui node
  int node_prev;
  int node_next;

  DepNode(int node, int data_node) :
      node(node),
      data_node(data_node),
      prev(-1),
      next(-1),
      node_prev(-1),
      node_next(-1) {}
};

class Tree {
public:
  template <typename T, bool IsConst>
  class Data_ {
    using data_ptr_t = std::conditional_t<IsConst, const T*, T*>;

  public:
    const T& operator*() const {
      tree->data_access(data_node);
      return *ptr;
    }
    const T* operator->() const {
      tree->data_access(data_node);
      return ptr;
    }
    T& mut() const {
      static_assert(!IsConst);
      tree->data_mutate(data_node);
      return *ptr;
    }
    bool modified() const {
      tree->data_access(data_node);
      return tree->data_nodes[data_node].modified;
    }

    template <
        bool OtherConst,
        typename = std::enable_if_t<IsConst || !OtherConst>>
    Data_(const Data_<T, OtherConst>& other) :
        tree(other.tree), data_node(other.data_node), ptr(other.ptr) {}

    template <
        bool OtherConst,
        typename = std::enable_if_t<IsConst || !OtherConst>>
    Data_(Data_<T, OtherConst>&& other) :
        tree(other.tree), data_node(other.data_node), ptr(other.ptr) {}

  private:
    Data_(Tree* tree, int data_node) :
        tree(tree),
        data_node(data_node),
        ptr(std::any_cast<T>(&tree->data_nodes[data_node].data)) {}

    Tree* tree;
    int data_node;
    data_ptr_t ptr;

    friend class Tree;
  };
  template <typename T>
  using Data = Data_<T, false>;
  template <typename T>
  using ConstData = Data_<T, true>;

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

    // TEMP: May remove if not needed
    int get_index() const {
      return index;
    }

    template <typename T>
    Data_<T, IsConst> data() const {
      return const_cast<Tree*>(tree)->template data_single<T>(index);
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

  Tree(const deinit_node_t& deinit_node) : deinit_node(deinit_node) {}

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

  void insert_next();
  void erase_this();
  void erase_next();

  template <typename T>
  Data<T> data_parent(const std::function<T()>& construct = []() {
    return T();
  }) {
    assert(parent_ != -1);
    int data_node;
    if (parent_data_current_ == -1) {
      data_node = nodes[parent_].first_data;
    } else {
      data_node = data_nodes[parent_data_current_].next;
    }
    if (data_node == -1) {
      if (!nodes[parent_].is_new) {
        throw WindowError("Changed the number of data nodes");
      }
      data_node = create_data_node(parent_);
      data_nodes[data_node].data = construct();
    }
    parent_data_current_ = data_node;
    return Data<T>(this, parent_data_current_);
  }

  template <typename T>
  Data<T> data_current(const std::function<T()>& construct = []() {
    return T();
  }) {
    assert(current_ != -1);
    return data_single(current_, construct);
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
  template <typename T>
  Data<T> data_single(
      int node,
      const std::function<T()>& construct = []() { return T(); }) {
    int data_node = nodes[node].first_data;
    if (data_node == -1) {
      if (!nodes[node].is_new) {
        throw WindowError("Changed the number of data nodes");
      }
      data_node = create_data_node(node);
      data_nodes[data_node].data = construct();
    }
    return Data<T>(this, data_node);
  }

  int create_node(int parent, int prev);
  void remove_node(int node);

  void set_needs_visit(int node, bool visit_children = false);

  int create_data_node(int node);
  void data_mutate(int data_node);
  void data_access(int data_node);

  void remove_data_node(int data_node);
  void remove_dep_node(int dep_node);
  void remove_node_data_nodes(int node);
  void remove_node_dep_nodes(int node);

  deinit_node_t deinit_node;
  VectorMap<Node> nodes;
  VectorMap<DataNode> data_nodes;
  VectorMap<DepNode> dep_nodes;

  bool is_new = true;
  int root_ = -1;
  int parent_ = -1;
  int current_ = -1;
  int parent_data_current_ = -1;
  std::stack<int> parent_data_current_stack_;
  std::vector<int> queue_needs_visit;
};

} // namespace datagui

#pragma once

#include "datagui/exception.hpp"
#include "datagui/tree/state.hpp"
#include "datagui/tree/vector_map.hpp"
#include <assert.h>
#include <functional>
#include <string>

namespace datagui {

enum class NodeType { Primitive, Data, Container, Optional, Variant };

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

  int dest_data_dep = -1;
  int source_data_dep = -1;

  bool is_new = true;
  bool needs_visit = true;
  bool modified = false;
  bool visible = true;

  State state;
};

struct DataDepNode {
  int source;
  int source_prev = -1;
  int source_next = -1;

  int dest;
  int dest_prev = -1;
  int dest_next = -1;

  DataDepNode(int source, int dest) : source(source), dest(dest) {}
};

class Tree {
public:
  template <bool IsConst>
  class Ptr_ {
    using tree_ptr_t = std::conditional_t<IsConst, const Tree*, Tree*>;
    using state_ptr_t = std::conditional_t<IsConst, const State*, State*>;
    using state_ref_t = std::conditional_t<IsConst, const State&, State&>;

  public:
    Ptr_ first_child() {
      assert(index != -1);
      return Ptr_(tree, tree->nodes[index].first_child);
    }
    Ptr_ next() {
      assert(index != -1);
      return Ptr_(tree, tree->nodes[index].next);
    }
    state_ref_t operator*() const {
      return tree->nodes[index].state;
    }
    state_ptr_t operator->() const {
      return &tree->nodes[index].state;
    }
    operator bool() const {
      return index != -1;
    }
    bool visible() const {
      return tree->nodes[index].visible;
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

  private:
    Ptr_(tree_ptr_t tree, int index) : tree(tree), index(index) {}

    tree_ptr_t tree;
    int index;

    friend class Tree;
  };
  using Ptr = Ptr_<false>;
  using ConstPtr = Ptr_<true>;

  template <typename T>
  class Data {
  public:
    const T& operator*() const {
      access();
      return *ptr;
    }
    const T* operator->() const {
      access();
      return ptr;
    }
    T& mut() const {
      tree->queue_changed_nodes.push_back(node);
    }
    operator bool() const {
      access();
      return tree->nodes[node].modified;
    }

  private:
    Data(Tree* tree, int node, T* ptr) : tree(tree), node(node), ptr(ptr) {
      if (tree->nodes[node].type != NodeType::Data) {
        throw WindowError("Tried to create a Data object for non-data node");
      }
    }

    void access() const {
      tree->add_data_dependency(node, tree->parent_);
    }

    Tree* tree;
    int node;
    T* ptr;

    friend class Tree;
  };

  using init_state_t = std::function<void(State& state)>;
  using deinit_state_t = std::function<void(const State&)>;

  Tree(const deinit_state_t& deinit_state) : deinit_state(deinit_state) {}

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
  Data<T> data(T* ptr) {
    if (nodes[current_].is_new) {
      nodes[current_].type = NodeType::Data;
    } else if (nodes[current_].type != NodeType::Data) {
      throw WindowError("Node not created as a data node");
    }
    return Data<T>(this, current_, ptr);
  }

  void set_modified(int node);

#if 0
  int root() const {
    return root_;
  }
  int parent() const {
    return parent_;
  }
  int current() const {
    return current_;
  }

  const Node& operator[](std::size_t i) const {
    return nodes[i];
  }
  Node& operator[](std::size_t i) {
    return nodes[i];
  }

  Ptr ptr(int index) {
    return Ptr(this, index);
  }
  ConstPtr ptr(int index) const {
    return ConstPtr(this, index);
  }
#else

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
#endif

private:
  int create_node(int parent, int prev);
  void remove_node(int node);
  void set_needs_visit(int node);
  void add_data_dependency(int source, int dest);
  void remove_data_dest_dependencies(int node);
  void remove_data_source_dependencies(int node);

  deinit_state_t deinit_state;
  VectorMap<Node> nodes;
  VectorMap<DataDepNode> data_dep_nodes;

  bool is_new = true;
  int root_ = -1;
  int parent_ = -1;
  int current_ = -1;
  std::vector<int> queue_changed_nodes;
};

} // namespace datagui

#pragma once

#include "datagui/exception.hpp"
#include "datagui/tree/state.hpp"
#include "datagui/tree/vector_map.hpp"
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
      tree->add_data_dependency(node, tree->parent());
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
  bool optional_down(bool open, const init_state_t& init_state);
  bool variant_down(const std::string& label, const init_state_t& init_state);

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

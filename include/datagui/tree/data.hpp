#pragma once

#include "datagui/tree/vector_map.hpp"
#include <any>

namespace datagui {

struct DataNode {
  int gui_node = -1;
  std::any data;
  bool modified = false;

  // Linked list of data nodes for a given gui node
  int prev = -1;
  int next = -1;
  int first_dep = -1;
};

struct DepNode {
  int gui_node = -1;
  bool data_modified = false;

  // Linked list of dependencies for a given data node
  int data_node = -1;
  int prev = -1;
  int next = -1;

  // Linked list of dependencies for a given gui node
  int gui_prev = -1;
  int gui_next = -1;
};

class DataTree {
public:
  template <typename T>
  class Data {
  public:
    const T& operator*() const {
      tree->data_accessed(data_node);
      return *ptr;
    }
    const T* operator->() const {
      tree->data_accessed(data_node);
      return ptr;
    }
    T& mut() const {
      tree->data_modified(data_node);
    }
    operator bool() const {
      tree->data_accessed(data_node);
      return tree->data_nodes[data_node].modified;
    }

  private:
    Data(DataTree* tree, int data_node, T* ptr) :
        tree(tree), data_node(data_node), ptr(ptr) {}

    void access() const {
      tree->add_data_dependency(node, tree->parent_);
    }

    DataTree* tree;
    int data_node;
    T* ptr;

    friend class Tree;
  };

  template <typename T>
  Data<T> operator[](int index) {
    T* ptr = nullptr; // TODO
    return Data<T>(this, index, ptr);
  }

template <typename T>
void add_data(int gui_node, )

    private : void data_modified(int data_node) {
    int dep = data_nodes[data_node].first_dep;
    while (dep != -1) {
      dep_nodes[dep].data_modified = true;
      dep = dep_nodes[dep].next;
    }
  }
  void data_accessed(int data_node) {}

  void add_link(int data_node, int dest_node);
  void remove_link(int data_node, int dest_node);

  VectorMap<DataNode> data_nodes;
  VectorMap<DepNode> dep_nodes;
};

} // namespace datagui

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
}
;

} // namespace datagui

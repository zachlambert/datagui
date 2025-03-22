#include "datagui/tree/tree.hpp"
#include "datagui/exception.hpp"
#include <assert.h>
#include <stack>

namespace datagui {

void Tree::begin() {
  parent_ = -1;
  current_ = -1;
}

void Tree::end() {
  if (parent_ != -1) {
    throw WindowError(
        "Didn't call layout_... and layout_end the same number of times");
  }

  is_new = false;
  if (root_ == -1) {
    return;
  }

  std::stack<int> stack;
  stack.push(root_);
  while (!stack.empty()) {
    int node = stack.top();
    stack.pop();

    nodes[node].is_new = false;
    nodes[node].needs_visit = false;
    nodes[node].modified = false;

    int child = nodes[node].first_child;
    while (child != -1) {
      stack.push(child);
      child = nodes[child].next;
    }
  }

  for (int node : queue_changed_nodes) {
    set_modified(node);
  }
  queue_changed_nodes.clear();
}

void Tree::container_next(const init_state_t& init_state) {
  if (parent_ == -1) {
    if (current_ != -1) {
      throw WindowError("Root node was visited twice");
    }
    if (root_ == -1) {
      root_ = create_node(-1, -1);
      init_state(nodes[root_].state);
    }
    current_ = root_;
    return;
  }

  if (nodes[parent_].type != NodeType::Container) {
    throw WindowError("Cannot call container_next in a non-container node");
  }

  int next;
  if (current_ != -1) {
    if (nodes[current_].parent == -1) {
      throw WindowError("Cannot call next on the root node more than once");
    }
    next = nodes[current_].next;
  } else {
    next = nodes[parent_].first_child;
  }

  if (next == -1) {
    if (!nodes[parent_].is_new) {
      throw WindowError(
          "Called next with no remaining nodes in this container");
    }
    current_ = create_node(parent_, current_);
    init_state(nodes[current_].state);
  } else {
    current_ = next;
  }
}

void Tree::up() {
  if (parent_ == -1) {
    throw WindowError("Called up too many times");
  }

  if (current_ == -1 && nodes[parent_].first_child != -1) {
    throw WindowError("Called up without visiting any children");
  }

  if (nodes[current_].next != -1) {
    throw WindowError("Called up without visiting all children");
  }

  current_ = parent_;
  parent_ = nodes[current_].parent;
}

bool Tree::container_down() {
  if (parent_ == -1 && is_new || nodes[parent_].is_new) {
    nodes[current_].type = NodeType::Container;
  } else if (nodes[current_].type != NodeType::Container) {
    throw WindowError("Node type changed, previously was container");
  }
  if (!nodes[current_].needs_visit) {
    return false;
  }
  parent_ = current_;
  current_ = -1;
  return true;
}

bool Tree::optional_down(
    bool open,
    const init_state_t& init_state,
    bool retain) {
  if (parent_ == -1 && is_new || nodes[parent_].is_new) {
    nodes[current_].type = NodeType::Optional;
  } else if (nodes[current_].type != NodeType::Optional) {
    throw WindowError("Node type changed, previously was optional");
  }

  if (nodes[current_].open && !open && !retain) {
    if (nodes[current_].first_child != -1) {
      remove_node(nodes[current_].first_child);
    }
    return false;
  }
  if (!nodes[current_].open && open) {
    set_needs_visit(current_);
  }

  nodes[current_].open = open;
  nodes[current_].visible = nodes[current_].open;

  if (!nodes[current_].needs_visit) {
    return false;
  }
  parent_ = current_;

  if (nodes[parent_].first_child == -1) {
    nodes[parent_].first_child = create_node(parent_, -1);
    init_state(nodes[nodes[parent_].first_child].state);
  }
  current_ = nodes[parent_].first_child;
  nodes[current_].needs_visit = nodes[parent_].needs_visit;

  return true;
}

bool Tree::variant_down(
    const std::string& label,
    const init_state_t& init_state,
    bool retain) {
  if (label.empty()) {
    throw WindowError("Variant label cannot be empty");
  }
  if (parent_ == -1 && is_new || nodes[parent_].is_new) {
    nodes[current_].type = NodeType::Variant;
  } else if (nodes[current_].type != NodeType::Variant) {
    throw WindowError("Node type changed, previously was optional");
  }

  bool switched = false;
  if (nodes[current_].open_label != label) {
    nodes[current_].open_label = label;
    set_needs_visit(current_);
    switched = true;
  }
  if (!nodes[current_].needs_visit) {
    return false;
  }

  int iter = nodes[current_].first_child;
  while (iter != -1) {
    assert(!nodes[iter].label.empty());
    if (nodes[iter].label == label) {
      break;
    }
    iter = nodes[iter].next;
  }

  if (iter != -1) {
    if (switched || nodes[iter].needs_visit) {
      parent_ = current_;
      current_ = iter;
      return true;
    }
    return false;
  }

  iter = create_node(current_, nodes[current_].last_child);
  init_state(nodes[iter].state);

  return true;
}

void Tree::insert_next() {
  create_node(parent_, current_);
}

void Tree::erase_this() {
  if (current_ == -1) {
    throw WindowError("Tried to call erase_prev() with no current node");
  }
  int new_current = nodes[current_].prev;
  remove_node(current_);
  current_ = new_current;
}

void Tree::erase_next() {
  int next =
      (current_ == -1) ? nodes[parent_].first_child : nodes[current_].next;
  remove_node(next);
}

void Tree::set_modified(int node) {
  nodes[node].modified = true;
  int iter = nodes[node].source_data_dep;
  while (iter != -1) {
    set_needs_visit(data_dep_nodes[iter].dest);
    iter = data_dep_nodes[iter].source_next;
  }
}

int Tree::create_node(int parent, int prev) {
  int node = nodes.emplace();
  nodes[node].parent = parent;

  if (parent_ == -1) {
    return node;
  }

  nodes[node].prev = prev;
  int next = (prev == -1) ? nodes[parent].first_child : nodes[prev].next;
  nodes[node].next = next;

  if (prev != -1) {
    nodes[prev].next = node;
  } else {
    nodes[parent].first_child = node;
  }
  if (next != -1) {
    nodes[next].prev = node;
  } else {
    nodes[parent].last_child = node;
  }
  return node;
}

void Tree::remove_node(int node) {
  std::stack<int> stack;
  stack.push(node);

  while (!stack.empty()) {
    int node_index = stack.top();
    const auto& node = nodes[node_index];
    if (node.first_child == -1) {
      stack.pop();
      if (node.prev != -1) {
        nodes[node.prev].next = node.next;
      } else if (node.parent != -1) {
        nodes[node.parent].first_child = node.next;
      }
      if (node.next != -1) {
        nodes[node.next].prev = node.prev;
      } else if (node.parent != -1) {
        nodes[node.parent].last_child = node.prev;
      }
      deinit_state(node.state);
      remove_data_dest_dependencies(node_index);
      remove_data_source_dependencies(node_index);
      nodes.pop(node_index);

      continue;
    }
    int iter = node.first_child;
    while (iter != -1) {
      stack.push(iter);
      iter = nodes[iter].next;
    }
  }
}

void Tree::set_needs_visit(int node) {
  int iter = node;
  while (iter != -1) {
    nodes[iter].needs_visit = true;
    iter = nodes[iter].parent;
  }
}

void Tree::add_data_dependency(int source, int dest) {
  // Check if the dependency exists already
  int iter = nodes[dest].dest_data_dep;
  while (iter != -1) {
    if (data_dep_nodes[iter].source == source) {
      return;
    }
    iter = data_dep_nodes[iter].dest_next;
  }

  // Create new node
  int new_node = data_dep_nodes.emplace(source, dest);

  // Insert at front of dest linked list
  data_dep_nodes[new_node].dest_next = nodes[dest].dest_data_dep;
  data_dep_nodes[nodes[dest].dest_data_dep].dest_prev = new_node;
  nodes[dest].dest_data_dep = new_node;

  // Insert at front of source linked list
  data_dep_nodes[new_node].source_next = nodes[source].source_data_dep;
  data_dep_nodes[nodes[dest].source_data_dep].source_prev = new_node;
  nodes[source].source_data_dep = new_node;
}

void Tree::remove_data_dest_dependencies(int node) {
  if (nodes[node].dest_data_dep == -1) {
    return;
  }
  const auto& dep_node = data_dep_nodes[nodes[node].dest_data_dep];
  if (dep_node.dest_prev != -1) {
    data_dep_nodes[dep_node.dest_prev].dest_next = dep_node.dest_next;
  }
  if (dep_node.dest_next != -1) {
    data_dep_nodes[dep_node.dest_next].dest_prev = dep_node.dest_prev;
  }
}

void Tree::remove_data_source_dependencies(int node) {
  if (nodes[node].source_data_dep == -1) {
    return;
  }
  const auto& dep_node = data_dep_nodes[nodes[node].source_data_dep];
  if (dep_node.source_prev != -1) {
    data_dep_nodes[dep_node.source_prev].source_next = dep_node.source_next;
  }
  if (dep_node.source_next != -1) {
    data_dep_nodes[dep_node.source_next].source_prev = dep_node.source_prev;
  }
}

} // namespace datagui

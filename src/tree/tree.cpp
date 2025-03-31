#include "datagui/tree/tree.hpp"
#include "datagui/exception.hpp"
#include <assert.h>
#include <stack>

namespace datagui {

void Tree::begin() {
  parent_ = -1;
  current_ = -1;
  data_current_ = -1;
}

void Tree::end() {
  if (parent_ != -1) {
    throw WindowError(
        "Didn't call layout_... and layout_end the same number of times");
  }
  if (!parent_data_current_.empty()) {
    // If this isn't true, should also fail the above condition
    assert(false);
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

    int child = nodes[node].first_child;
    while (child != -1) {
      stack.push(child);
      child = nodes[child].next;
    }
  }

  for (int node : queue_needs_visit) {
    set_needs_visit(node);
  }
  queue_needs_visit.clear();
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

  data_current_ = parent_data_current_.top();
  parent_data_current_.pop();
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
  parent_data_current_.push(data_current_);
  data_current_ = -1;
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

  parent_data_current_.push(data_current_);
  data_current_ = -1;

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
      parent_data_current_.push(data_current_);
      data_current_ = -1;
      return true;
    }
    return false;
  }

  iter = create_node(current_, nodes[current_].last_child);
  init_state(nodes[iter].state);

  parent_data_current_.push(data_current_);
  data_current_ = -1;

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
      remove_node_data_nodes(node_index);
      remove_node_dep_nodes(node_index);
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

int Tree::create_data_node(int node) {
  int new_node = data_nodes.emplace(node);

  int prev = nodes[node].first_data;
  if (prev == -1) {
    nodes[node].first_data = new_node;
    return new_node;
  }
  while (data_nodes[prev].next != -1) {
    prev = data_nodes[prev].next;
  }
  data_nodes[prev].next = new_node;
  data_nodes[new_node].prev = prev;
  return new_node;
}

void Tree::data_access(int data_node) {
  int dep_gui_node = parent_;

  // Check if the dependency exists already
  int dep = nodes[dep_gui_node].first_dep;
  while (dep != -1) {
    if (dep_nodes[dep].data_node == data_node) {
      return; // Already have a dependency to the given data_node
    }
    dep = dep_nodes[dep].next;
  }

  // Create new dependency
  int new_dep = dep_nodes.emplace(dep_gui_node, data_node);

  // Insert at front of dependencies linked list
  dep_nodes[new_dep].next = data_nodes[data_node].first_dep;
  if (data_nodes[data_node].first_dep != -1) {
    dep_nodes[data_nodes[data_node].first_dep].prev = new_dep;
  }
  data_nodes[data_node].first_dep = new_dep;

  // Insert at front of gui node linked list
  dep_nodes[new_dep].node_next = nodes[dep_gui_node].first_dep;
  if (nodes[dep_gui_node].first_dep != -1) {
    dep_nodes[nodes[dep_gui_node].first_dep].prev = new_dep;
  }
  nodes[dep_gui_node].first_dep = new_dep;
}

void Tree::data_mutate(int data_node) {
  data_nodes[data_node].modified = true;
  int dep = data_nodes[data_node].first_dep;
  while (dep != -1) {
    queue_needs_visit.push_back(dep_nodes[dep].node);
    dep = dep_nodes[dep].next;
  }
}

void Tree::remove_data_node(int data_node) {
  int dep = data_nodes[data_node].first_dep;
  while (dep != -1) {
    remove_dep_node(dep);
  }

  if (data_nodes[data_node].prev != -1) {
    data_nodes[data_nodes[data_node].prev].next = data_nodes[data_node].next;
  } else {
    nodes[data_nodes[data_node].node].first_data = data_nodes[data_node].next;
  }

  if (data_nodes[data_node].next != -1) {
    data_nodes[data_nodes[data_node].next].prev = data_nodes[data_node].prev;
  }

  data_nodes.pop(data_node);
}

void Tree::remove_dep_node(int dep_node) {
  if (dep_nodes[dep_node].prev != -1) {
    dep_nodes[dep_nodes[dep_node].prev].next = dep_nodes[dep_node].next;
  } else {
    data_nodes[dep_nodes[dep_node].data_node].first_dep =
        dep_nodes[dep_node].next;
  }
  if (dep_nodes[dep_node].next != -1) {
    dep_nodes[dep_nodes[dep_node].next].prev = dep_nodes[dep_node].prev;
  }

  if (dep_nodes[dep_node].node_prev != -1) {
    dep_nodes[dep_nodes[dep_node].node_prev].node_next =
        dep_nodes[dep_node].node_next;
  } else {
    nodes[dep_nodes[dep_node].node].first_dep = dep_nodes[dep_node].node_next;
  }
  if (dep_nodes[dep_node].node_next != -1) {
    dep_nodes[dep_nodes[dep_node].node_next].node_prev =
        dep_nodes[dep_node].node_prev;
  }

  dep_nodes.pop(dep_node);
}

void Tree::remove_node_data_nodes(int node) {
  int data = nodes[node].first_data;

  while (data != -1) {
    int next = data_nodes[data].next;
    remove_data_node(data);
    data = next;
  }
}

void Tree::remove_node_dep_nodes(int node) {
  int dep = nodes[node].first_dep;

  while (dep != -1) {
    int next = dep_nodes[dep].node_next;
    remove_dep_node(dep);
    dep = next;
  }
}

} // namespace datagui

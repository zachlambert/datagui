#include "datagui/tree/tree.hpp"
#include "datagui/exception.hpp"
#include <assert.h>
#include <stack>

namespace datagui {

Tree::Tree(const deinit_node_t& deinit_node) : deinit_node(deinit_node) {
  external_ = nodes.emplace();
  nodes[external_].type = NodeType::External;

  parent_ = external_;
  current_ = -1;
  parent_variable_current_ = -1;
}

void Tree::begin() {
  parent_ = -1;
  current_ = -1;
  parent_variable_current_ = -1;

  for (auto trigger : queue_triggered) {
    set_triggered(trigger.node);
    if (trigger.immutable) {
      reset_node(trigger.node);
    }
  }
  queue_triggered.clear();
}

void Tree::end() {
  if (parent_ != -1) {
    throw WindowError(
        "Didn't call layout_... and layout_end the same number of times");
  }
  if (!parent_variable_current_stack_.empty()) {
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
    nodes[node].triggered = false;

    int child = nodes[node].first_child;
    while (child != -1) {
      stack.push(child);
      child = nodes[child].next;
    }
  }

  parent_ = external_;
  parent_variable_current_ = -1;
}

void Tree::container_next(const init_state_t& init_state) {
  if (parent_ == -1) {
    if (current_ != -1) {
      throw WindowError("Root node was visited twice");
    }
    if (root_ == -1) {
      root_ = create_node(-1, -1);
    }
    if (nodes[root_].is_new) {
      init_node(root_, init_state);
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
  } else {
    current_ = next;
  }
  if (nodes[current_].is_new) {
    init_node(current_, init_state);
  }
}

void Tree::up() {
  if (parent_ == -1) {
    throw WindowError("Called up too many times");
  }

  if (current_ == -1 && nodes[parent_].first_child != -1) {
    throw WindowError("Called up without visiting any children");
  }

  if (current_ != -1 && nodes[current_].next != -1) {
    throw WindowError("Called up without visiting all children");
  }

  current_ = parent_;
  parent_ = nodes[current_].parent;

  parent_variable_current_ = parent_variable_current_stack_.top();
  parent_variable_current_stack_.pop();
}

bool Tree::container_down() {
  if (parent_ == -1 && is_new || nodes[parent_].is_new) {
    nodes[current_].type = NodeType::Container;
  } else if (nodes[current_].type != NodeType::Container) {
    throw WindowError("Node type changed, previously was container");
  }
  if (!nodes[current_].triggered) {
    return false;
  }
  parent_ = current_;
  current_ = -1;
  parent_variable_current_stack_.push(parent_variable_current_);
  parent_variable_current_ = -1;
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
    set_triggered(current_);
  }

  nodes[current_].open = open;
  nodes[current_].visible = nodes[current_].open;

  if (!nodes[current_].triggered) {
    return false;
  }
  parent_ = current_;

  if (nodes[parent_].first_child == -1) {
    nodes[parent_].first_child = create_node(parent_, -1);
  }
  current_ = nodes[parent_].first_child;
  nodes[current_].triggered = nodes[parent_].triggered;

  parent_variable_current_stack_.push(parent_variable_current_);
  parent_variable_current_ = -1;

  if (nodes[current_].is_new) {
    init_node(current_, init_state);
  }

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
    set_triggered(current_);
    switched = true;
  }
  if (!nodes[current_].triggered) {
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
    if (switched || nodes[iter].triggered) {
      parent_ = current_;
      current_ = iter;
      parent_variable_current_stack_.push(parent_variable_current_);
      parent_variable_current_ = -1;
      return true;
    }
    return false;
  }

  current_ = create_node(current_, nodes[current_].last_child);
  if (nodes[current_].is_new) {
    init_node(current_, init_state);
  }

  parent_variable_current_stack_.push(parent_variable_current_);
  parent_variable_current_ = -1;

  return true;
}

#if 0
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
#endif

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

void Tree::init_node(int node, const init_state_t& init_state) {
  variable_access_node_ = node;
  init_state(nodes[node].state);
  variable_access_node_ = -1;
}

void Tree::reset_node(int node) {
  nodes[node].version++;
  auto child = nodes[node].first_child;
  while (child != -1) {
    int next = nodes[child].next;
    remove_node(child);
    child = next;
  }
  deinit_node(ConstPtr(this, node));
  // remove_node_variable_nodes(node);
  // remove_node_dep_nodes(node);
  nodes[node].is_new = true;
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
      deinit_node(ConstPtr(this, node_index));
      remove_node_variable_nodes(node_index);
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

void Tree::set_triggered(int node) {
  // Trigger the node and all ancestors
  int iter = node;
  while (iter != -1) {
    nodes[iter].triggered = true;
    iter = nodes[iter].parent;
  }

  // Trigger descendants
  std::stack<int> stack;
  stack.push(node);
  while (!stack.empty()) {
    int node = stack.top();
    stack.pop();
    int child = nodes[node].first_child;
    while (child != -1) {
      nodes[child].triggered = true;
      if (nodes[child].first_child != -1) {
        stack.push(child);
      }
      child = nodes[child].next;
    }
  }
}

int Tree::create_variable_node(int node) {
  int new_node = variable_nodes.emplace(node);

  int prev = nodes[node].first_variable;
  if (prev == -1) {
    nodes[node].first_variable = new_node;
    return new_node;
  }
  while (variable_nodes[prev].next != -1) {
    prev = variable_nodes[prev].next;
  }
  variable_nodes[prev].next = new_node;
  variable_nodes[new_node].prev = prev;
  return new_node;
}

void Tree::variable_access(int variable_node) {
  int dep_gui_node =
      variable_access_node_ != -1 ? variable_access_node_ : parent_;
  if (dep_gui_node == -1) {
    return;
  }

  // Check if the dependency exists already
  int dep = nodes[dep_gui_node].first_dep;
  while (dep != -1) {
    if (dep_nodes[dep].variable_node == variable_node) {
      return; // Already have a dependency to the given variable_node
    }
    dep = dep_nodes[dep].next;
  }

  // Create new dependency
  int new_dep = dep_nodes.emplace(dep_gui_node, variable_node);

  // Insert at front of dependencies linked list
  dep_nodes[new_dep].next = variable_nodes[variable_node].first_dep;
  if (variable_nodes[variable_node].first_dep != -1) {
    dep_nodes[variable_nodes[variable_node].first_dep].prev = new_dep;
  }
  variable_nodes[variable_node].first_dep = new_dep;

  // Insert at front of gui node linked list
  dep_nodes[new_dep].node_next = nodes[dep_gui_node].first_dep;
  if (nodes[dep_gui_node].first_dep != -1) {
    dep_nodes[nodes[dep_gui_node].first_dep].node_prev = new_dep;
  }
  nodes[dep_gui_node].first_dep = new_dep;
}

void Tree::variable_mutate(int variable_node) {
  variable_nodes[variable_node].modified = true;
  int dep = variable_nodes[variable_node].first_dep;
  while (dep != -1) {
    queue_triggered.emplace_back(dep_nodes[dep].node, true);
    dep = dep_nodes[dep].next;
  }
}

void Tree::remove_variable_node(int variable_node) {
  int dep = variable_nodes[variable_node].first_dep;
  while (dep != -1) {
    int next = dep_nodes[dep].node_next;
    remove_dep_node(dep);
    dep = next;
  }

  if (variable_nodes[variable_node].prev != -1) {
    variable_nodes[variable_nodes[variable_node].prev].next =
        variable_nodes[variable_node].next;
  } else {
    nodes[variable_nodes[variable_node].node].first_variable =
        variable_nodes[variable_node].next;
  }

  if (variable_nodes[variable_node].next != -1) {
    variable_nodes[variable_nodes[variable_node].next].prev =
        variable_nodes[variable_node].prev;
  }

  variable_nodes.pop(variable_node);
}

void Tree::remove_dep_node(int dep_node) {
  if (dep_nodes[dep_node].prev != -1) {
    dep_nodes[dep_nodes[dep_node].prev].next = dep_nodes[dep_node].next;
  } else {
    variable_nodes[dep_nodes[dep_node].variable_node].first_dep =
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

void Tree::remove_node_variable_nodes(int node) {
  int data = nodes[node].first_variable;

  while (data != -1) {
    int next = variable_nodes[data].next;
    remove_variable_node(data);
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

std::string Tree::node_debug(int node) const {
  std::string result = "";
  result += "Node: " + std::to_string(node);
  result += "\nVersion: " + std::to_string(nodes[node].version);
  if (nodes[node].first_variable != -1) {
    result += "\nVariables:";
    for (int var = nodes[node].first_variable; var != -1;
         var = variable_nodes[var].next) {
      result += " " + std::to_string(var) + ", ";
    }
  }
  if (nodes[node].first_dep != -1) {
    result += "\nDeps:";
    for (int dep = nodes[node].first_dep; dep != -1;
         dep = dep_nodes[dep].node_next) {
      result += " " + std::to_string(dep_nodes[dep].variable_node) + ", ";
    }
  }
  return result;
}

} // namespace datagui

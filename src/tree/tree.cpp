#include "datagui/tree/tree.hpp"
#include <assert.h>
#include <stack>

namespace datagui {

void Tree::begin() {
  render_in_progress = true;
  parent_ = -1;
  current_ = -1;
  variable_current_ = -1;

  for (auto element : queue_revisit_) {
    set_revisit(element);
  }
  queue_revisit_.clear();

  for (auto variable : modified_variables_) {
    variables[variable].modified = false;
  }
  modified_variables_.clear();
  for (auto variable : queue_rerender_variables_) {
    set_rerender(variables[variable].element);
    variables[variable].data = std::move(variables[variable].data_new);
    assert(variables[variable].data);
    variables[variable].modified = true;
    modified_variables_.push_back(variable);
  }
  queue_rerender_variables_.clear();

  for (auto element : queue_remove_) {
    remove_element(element);
  }
  queue_remove_.clear();
}

void Tree::end() {
  if (parent_ != -1) {
    throw UsageError(
        "Didn't call layout_... and layout_end the same number of times");
  }
  if (!variable_stack_.empty()) {
    // If this isn't true, should also fail the above condition
    assert(false);
  }

  is_new = false;
  if (root_ == -1) {
    return;
  }

  // Clear the rerender and revisit flags
  // TODO: Can this be done during the visit?

  std::stack<int> stack;
  stack.push(root_);
  while (!stack.empty()) {
    int element = stack.top();
    stack.pop();

    elements[element].is_new = false;
    elements[element].revisit = false;
    elements[element].rerender = false;

    int child = elements[element].first_child;
    while (child != -1) {
      stack.push(child);
      child = elements[child].next;
    }
  }

  render_in_progress = false;
  parent_ = -1;
  variable_current_ = -1;
}

Element Tree::next(int type, const std::string& key) {
  if (parent_ == -1) {
    if (current_ != -1) {
      throw UsageError("Cannot create more than one root node");
    }
    if (root_ == -1) {
      root_ = create_element(-1, -1, type, "");
    }
    current_ = root_;
    return Element(this, current_);
  }

  if (current_ != -1 && elements[current_].parent == -1) {
    throw UsageError("Cannot call next on the root node more than once");
  }

  int prev = current_;
  int next =
      current_ == -1 ? elements[parent_].first_child : elements[current_].next;

  while (next != -1) {
    if (elements[next].key == key) {
      break;
    }
    if (!elements[parent_].rerender) {
      throw UsageError("Structure changed outside of a re-render");
    }
    if (elements[next].key.empty() && !key.empty()) {
      next = create_element(parent_, prev, type, key);
      break;
    }
    if (!elements[parent_].retain) {
      queue_remove_.push_back(next);
    }
    prev = next;
    next = elements[next].next;
  }

  if (next != -1) {
    if (type != elements[next].type && !elements[next].rerender) {
      throw UsageError("Structure changed outside of a re-render");
    }
    current_ = next;
    return Element(this, current_);
  }

  if (!elements[parent_].rerender) {
    throw UsageError("Structure changed outside of a re-render");
  }
  current_ = create_element(parent_, prev, type, key);
  return Element(this, current_);
}

bool Tree::down_if() {
  if (!elements[current_].revisit) {
    return false;
  }
  parent_ = current_;
  current_ = -1;
  variable_stack_.push(variable_current_);
  variable_current_ = -1;
  return true;
}

void Tree::down() {
  parent_ = current_;
  current_ = -1;
  variable_stack_.push(variable_current_);
  variable_current_ = -1;
}

void Tree::up() {
  if (parent_ == -1) {
    throw UsageError("Called up too many times");
  }

  int remove_from =
      current_ == -1 ? elements[parent_].first_child : elements[current_].next;

  if (remove_from != -1 && !elements[parent_].rerender) {
    throw UsageError("Structure changed outside of a re-render");
  }
  while (remove_from != -1) {
    queue_remove_.push_back(remove_from);
    remove_from = elements[remove_from].next;
  }

  current_ = parent_;
  parent_ = elements[current_].parent;

  variable_current_ = variable_stack_.top();
  variable_stack_.pop();
}

int Tree::create_element(
    int parent,
    int prev,
    int type,
    const std::string& key) {
  int data_index = 1;
  if (type != -1) {
    assert(type < data_containers.size());
    data_index = data_containers[type]->emplace();
  }

  int element = elements.emplace(type, data_index);
  auto& node = elements[element];
  node.parent = parent;
  node.key = key;

  node.prev = prev;
  int next = (prev == -1) ? parent == -1 ? root_ : elements[parent].first_child
                          : elements[prev].next;
  node.next = next;

  if (prev != -1) {
    elements[prev].next = element;
  } else {
    elements[parent].first_child = element;
  }
  if (next != -1) {
    elements[next].prev = element;
  } else {
    elements[parent].last_child = element;
  }
  return element;
}

void Tree::remove_element(int element) {
  std::stack<int> stack;
  stack.push(element);

  while (!stack.empty()) {
    int element = stack.top();
    const auto& node = elements[element];

    if (node.first_child == -1) {
      stack.pop();
      if (node.prev != -1) {
        elements[node.prev].next = node.next;
      } else if (node.parent != -1) {
        elements[node.parent].first_child = node.next;
      }
      if (node.next != -1) {
        elements[node.next].prev = node.prev;
      } else if (node.parent != -1) {
        elements[node.parent].last_child = node.prev;
      }

      if (node.data_index != -1) {
        data_containers[int(node.type)]->pop(node.data_index);
      }

      int variable = node.first_variable;
      while (variable != -1) {
        int next = variables[variable].next;
        remove_variable(variable);
        variable = next;
      }
      elements.pop(element);

      continue;
    }
    int iter = node.first_child;
    while (iter != -1) {
      stack.push(iter);
      iter = elements[iter].next;
    }
  }
}

int Tree::create_variable(int element) {
  int variable = variables.emplace(element);

  int prev;
  if (element == -1) {
    if (external_first_variable_ == -1) {
      external_first_variable_ = variable;
      return variable;
    }
    prev = external_first_variable_;
  } else {
    if (elements[element].first_variable == -1) {
      elements[element].first_variable = variable;
      return variable;
    }
    prev = elements[element].first_variable;
  }

  while (variables[prev].next != -1) {
    prev = variables[prev].next;
  }
  variables[prev].next = variable;
  variables[variable].prev = prev;
  return variable;
}

void Tree::remove_variable(int variable) {
  auto& node = variables[variable];
  if (node.prev != -1) {
    variables[node.prev].next = node.next;
  } else {
    elements[node.element].first_variable = node.next;
  }

  if (node.next != -1) {
    variables[node.next].prev = node.prev;
  }

  variables.pop(variable);
}

void Tree::variable_mutate(int variable) {
  queue_rerender_variables_.push_back(variable);
}

void Tree::set_revisit(int element) {
  if (element == -1) {
    return;
  }

  // Set revisit = true on the node and all it's ancestors
  int iter = element;
  while (iter != -1) {
    elements[iter].revisit = true;
    iter = elements[iter].parent;
  }
}

void Tree::set_rerender(int element) {
  // Revisit this node
  set_revisit(element);

  // Set revisit = true and rerender = true on all the descendants
  std::stack<int> stack;
  stack.push(element);
  while (!stack.empty()) {
    int element = stack.top();
    stack.pop();
    if (element == -1) {
      stack.push(root_);
      continue;
    }

    elements[element].revisit = true;
    elements[element].rerender = true;
    elements[element].version++;

    int child = elements[element].first_child;
    while (child != -1) {
      stack.push(child);
      child = elements[child].next;
    }
  }
}

std::string Tree::element_debug(int element) const {
  std::string result = "";
  result += "Element: " + std::to_string(element);
  result += "\nVersion: " + std::to_string(elements[element].version);

  int variable = elements[element].first_variable;
  if (variable != -1) {
    result += "\nVariables:";
    while (variable != -1) {
      result += " " + std::to_string(variable) + ", ";
      variable = variables[variable].next;
    }
  }
  return result;
}

} // namespace datagui

#include "datagui/tree/tree.hpp"
#include "datagui/log.hpp"
#include <assert.h>
#include <stack>

namespace datagui {

bool Tree::begin() {
  assert(parent_ == -1);
  assert(current_ == -1);
  assert(variable_stack_.empty());

  if (active_) {
    throw UsageError(
        "Didn't call end() after a previous begin() call returned true");
  }

  if (root_ == -1) {
    DATAGUI_LOG("Tree::begin", "BEGIN");
    DATAGUI_LOG_INDENT(1);
    active_ = true;
    return true;
  }

  // Clear revisit flags

  std::stack<int> stack;
  stack.push(root_);
  while (!stack.empty()) {
    int element = stack.top();
    auto& element_d = elements[element];
    stack.pop();

    if (!element_d.revisit) {
      // Element and descendents have revisit=false,
      // no need to continue down this subtree
      continue;
    }
    element_d.revisit = false;

    int child = element_d.first_child;
    while (child != -1) {
      stack.push(child);
      child = elements[child].next;
    }
  }

  // Applied queued revisit flags

  for (auto element : queue_revisit_) {
    DATAGUI_LOG("Tree::begin", "Element revisit: %i", element);
    set_revisit(element);
  }
  queue_revisit_.clear();

  // Clear variable modified flags

  for (int variable : modified_variables_) {
    variables[variable].modified = false;
    variables[variable].modified_internal = false;
  }
  modified_variables_.clear();

  // Apply queued variable modified flags

  for (auto [variable, internal] : queue_modified_variables_) {
    auto& variable_d = variables[variable];
    DATAGUI_LOG(
        "Tree::begin",
        "Variable modify: element=%i var=%i",
        variable_d.element,
        variable);

    // Revisit node
    if (variable_d.element == -1) {
      set_revisit(root_);
    } else {
      set_revisit(variable_d.element);
    }

    // Apply new data
    variable_d.data = std::move(variable_d.data_new);
    variable_d.modified = true;
    variable_d.modified_internal = internal;
    assert(variable_d.data);

    // Keep track of what is modified, to clear next iteration
    modified_variables_.push_back(variable);
  }
  queue_modified_variables_.clear();

  if (elements[root_].revisit) {
    DATAGUI_LOG("Tree::begin", "BEGIN");
    DATAGUI_LOG_INDENT(1);
    active_ = true;
    return true;
  }
  return false;
}

void Tree::end() {
  if (!active_) {
    throw UsageError("Called end() without calling begin() previously and "
                     "having it return true");
  }
  active_ = false;
  DATAGUI_LOG_INDENT(-1);
  DATAGUI_LOG("Tree::end", "END");

  if (parent_ != -1) {
    throw UsageError("Didn't call down and up the same number of times");
  }
  assert(variable_stack_.empty());

  assert(parent_ == -1);
  assert(current_ == root_);
  current_ = -1;
}

ElementPtr Tree::next(int id) {
  DATAGUI_LOG("Tree::next", "NEXT: current=%i id=%i", current_, id);

  if (parent_ == -1) {
    if (current_ != -1) {
      throw UsageError("Cannot call next more than once at the root");
    }
    if (root_ == -1) {
      root_ = create_element(-1, -1, id);
      DATAGUI_LOG("Tree::next", "Created root: element=%i id=%i", root_, id);
    } else if (elements[root_].id != id) {
      throw UsageError("TODO: Handle overwriting root");
    }
    current_ = root_;

  } else {

    int prev = current_;
    int iter = current_ == -1 ? elements[parent_].first_child
                              : elements[current_].next;
    while (iter != -1) {
      if (elements[iter].id == id) {
        break;
      }
      int to_remove = iter;
      iter = elements[iter].next;
      remove_element(to_remove);
    }

    if (iter == -1) {
      current_ = create_element(parent_, prev, id);
      DATAGUI_LOG(
          "Tree::next",
          "Created element: element=%i id=%i",
          current_,
          id);
    } else {
      current_ = iter;
    }
  }

  return ElementPtr(this, current_);
}

bool Tree::down_if() {
  if (!elements[current_].revisit) {
    return false;
  }
  DATAGUI_LOG("Tree::down", "DOWN (if): element=%i", current_);
  parent_ = current_;
  current_ = -1;

  if (variable_current_ != -1) {
    DATAGUI_LOG("Tree::down", "pushed variable %i", variable_current_);
  }
  DATAGUI_LOG_INDENT(1);

  variable_stack_.push(variable_current_);
  variable_current_ = -1;
  depth++;
  return true;
}

void Tree::down() {
  DATAGUI_LOG("Tree::down", "DOWN (force): element=%i", current_);
  parent_ = current_;
  current_ = -1;

  if (variable_current_ != -1) {
    DATAGUI_LOG("Tree::down", "pushed variable %i", variable_current_);
  }
  DATAGUI_LOG_INDENT(1);

  variable_stack_.push(variable_current_);
  variable_current_ = -1;
  depth++;
}

void Tree::up() {
  depth--;
  assert(depth >= 0);
  DATAGUI_LOG_INDENT(-1);

  if (parent_ == -1) {
    throw UsageError("Called up too many times");
  }

  int remove_from =
      current_ == -1 ? elements[parent_].first_child : elements[current_].next;

  while (remove_from != -1) {
    int next = elements[remove_from].next;
    remove_element(remove_from);
    remove_from = next;
  }

  current_ = parent_;
  parent_ = elements[current_].parent;
  DATAGUI_LOG("Tree::up", "UP: element=%i", current_);

  variable_current_ = variable_stack_.top();
  variable_stack_.pop();
  if (variable_current_ != -1) {
    DATAGUI_LOG("Tree::up", "popped variable %i", variable_current_);
  }
}

int Tree::create_element(int parent, int prev, int id) {
  int element = elements.emplace();
  auto& node = elements[element];
  node.parent = parent;
  node.id = id;

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

  // Initialise as modified
  variables[variable].modified = true;
  modified_variables_.push_back(variable);

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

void Tree::set_revisit(int element) {
  if (element == -1) {
    return;
  }

  // Set revisit = true on element and descendents
  std::stack<int> stack;
  stack.push(element);
  while (!stack.empty()) {
    int element = stack.top();
    stack.pop();

    if (elements[element].revisit) {
      // Revisit is already set to true, no need to update
      continue;
    }
    elements[element].revisit = true;

    int child = elements[element].first_child;
    while (child != -1) {
      stack.push(child);
      child = elements[child].next;
    }
  }

  // Set revisit = true on anscestors
  int iter = elements[element].parent;
  while (iter != -1) {
    if (elements[iter].revisit) {
      // Revisit already set here, must mean already set
      // for ancestors too
      break;
    }
    elements[iter].revisit = true;
    iter = elements[iter].parent;
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

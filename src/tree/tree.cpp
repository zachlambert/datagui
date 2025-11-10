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

  auto now = clock_t::now();
  for (const auto& dep : dependencies) {
    bool revisit;
    if (auto type = std::get_if<DependencyVar>(&dep.dependency)) {
      revisit = variables[type->variable].version != type->version;
    } else if (auto type = std::get_if<DependencyCondition>(&dep.dependency)) {
      revisit = type->condition();
    } else if (auto type = std::get_if<DependencyTimeout>(&dep.dependency)) {
      revisit = type->timepoint >= now;
    }
    if (revisit) {
      set_revisit(dep.element);
    }
  }

  if (elements[root_].revisit) {
    elements[root_].revisit = false;
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
  if (variables.size() > 0) {
    assert(variables[0].data);
  }

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
  elements[current_].revisit = false;

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

  // Dependencies must be re-created each time
  clear_dependencies(parent_);

  return true;
}

void Tree::down() {
  elements[current_].revisit = false;

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

  // Dependencies must be re-created each time
  clear_dependencies(parent_);
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

      clear_variables(element);
      clear_dependencies(element);

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

void Tree::clear_variables(int element) {
  int var = elements[element].first_variable;
  while (var != -1) {
    int next = variables[var].next;
    variables.pop(var);
    var = next;
  }
}

void Tree::create_dependency(int element, const Dependency& value) {
  int dep = dependencies.emplace(element, value);

  int first_dep = elements[element].first_dependency;
  if (first_dep == -1) {
    elements[element].first_dependency = dep;
  } else {
    assert(dependencies[first_dep].element == element);
    dependencies[dep].next = first_dep;
    dependencies[first_dep].prev = dep;
  }
}

void Tree::clear_dependencies(int element) {
  int dep = elements[element].first_dependency;
  while (dep != -1) {
    int next = variables[dep].next;
    dependencies.pop(dep);
    dep = next;
  }
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

  int variable = elements[element].first_variable;
  if (variable != -1) {
    result += "\nVars:";
    while (variable != -1) {
      result += " " + std::to_string(variable) + ", ";
      variable = variables[variable].next;
    }
  }
  return result;
}

int Tree::get_variable(UniqueAny&& value) {
  int variable;

  if (parent_ == -1) {
    // Vars are "external" - not within any container element
    if (variable_current_ == -1) {
      variable = external_first_variable_;
    } else {
      variable = variables[external_first_variable_].next;
    }
  } else {
    // Vars are within a container element
    if (variable_current_ == -1) {
      variable = elements[parent_].first_variable;
    } else {
      variable = variables[variable_current_].next;
    }
  }

  if (variable == -1) {
    if (parent_ == -1) {
      variable = create_variable(-1);
      variables[variable].data = std::move(value);
      DATAGUI_LOG("Tree::variable", "Created external variable: %i", variable);
    } else {
      variable = create_variable(parent_);
      variables[variable].data = std::move(value);
      DATAGUI_LOG("Tree::variable", "Created internal variable: %i", variable);
    }
  }

  variable_current_ = variable;
  return variable;
}

} // namespace datagui

#include "datagui/tree/tree.hpp"
#include <assert.h>
#include <stack>

namespace datagui {

static int g_next_id_ = 0;

int generate_id() {
  return g_next_id_++;
}

void Tree::poll() {
  auto now = clock_t::now();
  for (auto& dep : dependencies) {
    bool dirty;
    if (auto type = std::get_if<DependencyVar>(&dep.dependency)) {
      dirty = variables[type->variable].version != type->version;
      type->version = variables[type->variable].version;
    } else if (auto type = std::get_if<DependencyCondition>(&dep.dependency)) {
      dirty = type->condition();
    } else if (auto type = std::get_if<DependencyTimeout>(&dep.dependency)) {
      dirty = type->timepoint >= now;
    }
    if (dirty) {
      set_dirty(dep.element);
    }
  }
}

int Tree::create_element(int parent, int prev, int id, PropsType type) {
  int element = elements.emplace();
  auto& node = elements[element];
  node.parent = parent;
  node.id = id;
  node.props_type = type;
  node.props_index = emplace_props(type);

  node.prev = prev;
  int next = (prev == -1) ? parent == -1 ? root_ : elements[parent].first_child
                          : elements[prev].next;
  node.next = next;

  if (prev != -1) {
    elements[prev].next = element;
  } else if (parent != -1) {
    elements[parent].first_child = element;
  }
  if (next != -1) {
    elements[next].prev = element;
  } else if (parent != -1) {
    elements[parent].last_child = element;
  }
  return element;
}

void Tree::reset_element(int element, int id, PropsType type) {
  auto& node = elements[element];
  node.id = id;
  pop_props(node.props_type, node.props_index);
  node.props_type = type;
  node.props_index = emplace_props(type);
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

      pop_props(elements[element].props_type, elements[element].props_index);
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

int Tree::emplace_props(PropsType type) {
  switch (type) {
  case PropsType::Button:
    return button_props.emplace();
  case PropsType::Checkbox:
    return checkbox_props.emplace();
  case PropsType::Dropdown:
    return dropdown_props.emplace();
  case PropsType::Floating:
    return floating_props.emplace();
  case PropsType::Labelled:
    return labelled_props.emplace();
  case PropsType::Section:
    return section_props.emplace();
  case PropsType::Series:
    return series_props.emplace();
  case PropsType::TextBox:
    return text_box_props.emplace();
  case PropsType::TextInput:
    return text_input_props.emplace();
  default:
    assert(false);
    return -1;
  }
}

void Tree::pop_props(PropsType type, std::size_t index) {
  switch (type) {
  case PropsType::Button:
    button_props.pop(index);
    break;
  case PropsType::Checkbox:
    checkbox_props.pop(index);
    break;
  case PropsType::Dropdown:
    dropdown_props.pop(index);
    break;
  case PropsType::Floating:
    floating_props.pop(index);
    break;
  case PropsType::Labelled:
    labelled_props.pop(index);
    break;
  case PropsType::Section:
    section_props.pop(index);
    break;
  case PropsType::Series:
    series_props.pop(index);
    break;
  case PropsType::TextBox:
    text_box_props.pop(index);
    break;
  case PropsType::TextInput:
    text_input_props.pop(index);
    break;
  default:
    assert(false);
  }
}

int Tree::create_variable(int element) {
  int variable = variables.emplace(element);

  int prev;
  if (elements[element].first_variable == -1) {
    elements[element].first_variable = variable;
    return variable;
  }
  prev = elements[element].first_variable;

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
  elements[element].first_variable = -1;
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
    int next = dependencies[dep].next;
    dependencies.pop(dep);
    dep = next;
  }
  elements[element].first_dependency = -1;
}

void Tree::set_dirty(int element) {
  if (element == -1) {
    return;
  }

  // Set dirty = true on element and descendents
  std::stack<int> stack;
  stack.push(element);
  while (!stack.empty()) {
    int element = stack.top();
    stack.pop();

    if (elements[element].dirty) {
      // dirty is already set to true, no need to update
      continue;
    }
    elements[element].dirty = true;

    int child = elements[element].first_child;
    while (child != -1) {
      stack.push(child);
      child = elements[child].next;
    }
  }

  // Set dirty = true on anscestors
  int iter = elements[element].parent;
  while (iter != -1) {
    if (elements[iter].dirty) {
      // dirty already set here, must mean already set
      // for ancestors too
      break;
    }
    elements[iter].dirty = true;
    iter = elements[iter].parent;
  }
}

} // namespace datagui

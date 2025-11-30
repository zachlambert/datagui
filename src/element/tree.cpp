#include "datagui/element/tree.hpp"
#include <assert.h>
#include <stack>

namespace datagui {

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
      assert(dep.element != -1);
      set_dirty(dep.element);
    }
  }
}

void Tree::clear_dirty() {
  for (auto& element : elements) {
    element.dirty = false;
  }
}

int Tree::create_element(int parent, int prev, std::size_t id, Type type) {
  int element = elements.emplace();
  auto& node = elements[element];
  node.parent = parent;
  node.id = id;
  node.type = type;
  node.type_index = emplace_type(type);

  node.prev = prev;
  int next;
  if (prev != -1) {
    next = elements[prev].next;
  } else if (parent != -1) {
    next = elements[parent].first_child;
  } else {
    assert(root_ == -1);
    assert(prev == -1);
    root_ = element;
    next = -1;
  }
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

void Tree::reset_element(int element, std::size_t id, Type type) {
  auto& node = elements[element];
  node.id = id;
  pop_type(node.type, node.type_index);
  node.type = type;
  node.type_index = emplace_type(type);

  // Clear children
  remove_element(element, true);
}

void Tree::remove_element(int element, bool children_only) {
  std::stack<int> stack;
  stack.push(element);

  while (!stack.empty()) {
    int element = stack.top();
    const auto& node = elements[element];

    if (node.first_child == -1) {
      stack.pop();
      if (children_only && stack.empty()) {
        return;
      }

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

      pop_type(elements[element].type, elements[element].type_index);
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

int Tree::emplace_type(Type type) {
  switch (type) {
  case Type::Button:
    return button.emplace();
  case Type::Checkbox:
    return checkbox.emplace();
  case Type::ColorPicker:
    return color_picker.emplace();
  case Type::Dropdown:
    return dropdown.emplace();
  case Type::Floating:
    return floating.emplace();
  case Type::Labelled:
    return labelled.emplace();
  case Type::Section:
    return section.emplace();
  case Type::Series:
    return series.emplace();
  case Type::Slider:
    return slider.emplace();
  case Type::TextBox:
    return text_box.emplace();
  case Type::TextInput:
    return text_input.emplace();
  case Type::ViewportPtr:
    return viewport.emplace();
  default:
    assert(false);
    return -1;
  }
}

void Tree::pop_type(Type type, std::size_t index) {
  switch (type) {
  case Type::Button:
    button.pop(index);
    break;
  case Type::Checkbox:
    checkbox.pop(index);
    break;
  case Type::ColorPicker:
    color_picker.pop(index);
    break;
  case Type::Dropdown:
    dropdown.pop(index);
    break;
  case Type::Floating:
    floating.pop(index);
    break;
  case Type::Labelled:
    labelled.pop(index);
    break;
  case Type::Section:
    section.pop(index);
    break;
  case Type::Series:
    series.pop(index);
    break;
  case Type::Slider:
    slider.pop(index);
    break;
  case Type::TextBox:
    text_box.pop(index);
    break;
  case Type::TextInput:
    text_input.pop(index);
    break;
  case Type::ViewportPtr:
    viewport.pop(index);
    break;
  default:
    assert(false);
  }
}

int Tree::create_variable(int element) {
  int variable = variables.emplace(element);

  int prev;
  if (element == -1) {
    if (external_var_ == -1) {
      external_var_ = variable;
      return variable;
    }
    prev = external_var_;
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

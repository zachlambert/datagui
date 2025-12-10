#include "datagui/element/tree.hpp"
#include <assert.h>
#include <stack>

namespace datagui {

void Tree::poll() {
  auto now = clock_t::now();

  for (auto& dep : dependencies) {
    if (!dep.valid) {
      continue;
    }
    assert(variables.contains(dep.variable));
    bool dirty = (variables[dep.variable].version != dep.version);
    if (dirty) {
      assert(dep.element != -1);
      set_dirty(dep.element);
    }
  }
  auto iter = retrigger_elements.begin();
  while (iter != retrigger_elements.end()) {
    auto next = std::next(iter);
    if (*iter) {
      iter->set_dirty();
    } else {
      retrigger_elements.erase(iter);
    }
    iter = next;
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
#define HANDLE(Name, name) \
  case Type::Name: \
    return name.emplace();

  switch (type) {
    HANDLE(Button, button);
    HANDLE(Checkbox, checkbox);
    HANDLE(Collapsable, collapsable);
    HANDLE(ColorPicker, color_picker);
    HANDLE(Dropdown, dropdown);
    HANDLE(Group, group);
    HANDLE(Popup, popup);
    HANDLE(Select, select);
    HANDLE(Slider, slider);
    HANDLE(Split, split);
    HANDLE(Tabs, tabs);
    HANDLE(TextBox, text_box);
    HANDLE(TextInput, text_input);
    HANDLE(ViewportPtr, viewport);
  default:
    assert(false);
    return -1;
  }

#undef HANDLE
}

void Tree::pop_type(Type type, std::size_t index) {
#define HANDLE(Name, name) \
  case Type::Name: \
    name.pop(index); \
    break;

  switch (type) {
    HANDLE(Button, button);
    HANDLE(Checkbox, checkbox);
    HANDLE(Collapsable, collapsable);
    HANDLE(ColorPicker, color_picker);
    HANDLE(Dropdown, dropdown);
    HANDLE(Group, group);
    HANDLE(Popup, popup);
    HANDLE(Select, select);
    HANDLE(Slider, slider);
    HANDLE(Split, split);
    HANDLE(Tabs, tabs);
    HANDLE(TextBox, text_box);
    HANDLE(TextInput, text_input);
    HANDLE(ViewportPtr, viewport);
  default:
    assert(false);
    break;
  }

#undef HANDLE
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
    for (auto& dep : dependencies) {
      if (dep.variable == var) {
        dep.valid = false;
        break;
      }
    }
    int next = variables[var].next;
    variables.pop(var);
    var = next;
  }
  elements[element].first_variable = -1;
}

void Tree::create_dependency(int element, int variable) {
  int iter = elements[element].first_dependency;
  int prev = -1;
  while (iter != -1) {
    if (dependencies[iter].variable == variable) {
      return;
    }
    prev = iter;
    iter = dependencies[iter].next;
  }
  int current_version = variables[variable].version;
  int dep = dependencies.emplace(element, variable, current_version);
  dependencies[dep].prev = prev;
  if (prev == -1) {
    elements[element].first_dependency = dep;
  } else {
    dependencies[prev].next = dep;
  }
}

void Tree::clear_dependencies(int element) {
  assert(elements.contains(element));
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

  int iter = element;
  while (iter != -1) {
    if (elements[iter].dirty) {
      // If already true, must also be true for ancestors
      break;
    }
    elements[iter].dirty = true;
    iter = elements[iter].parent;
  }
}

} // namespace datagui

#include "datagui/gui.hpp"
#include <stack>

namespace datagui {

using namespace std::placeholders;

Gui::Gui(const Window::Config& config) :
    window(config),
    text_renderer(font_manager),
    linear_layout_system(geometry_renderer),
    text_system(font_manager, text_renderer),
    text_input_system(font_manager, text_renderer, geometry_renderer),
    tree(std::bind(&Gui::deinit_node, this, std::placeholders::_1)) {
  geometry_renderer.init();
  text_renderer.init();
}

bool Gui::running() const {
  return window.running();
}

bool Gui::linear_layout(
    const std::function<void(LinearLayoutStyle&)>& set_style) {
  tree.container_next([&](State& state) {
    state.element_type = ElementType::LinearLayout;
    state.element_index = linear_layout_system.emplace();
    if (set_style) {
      set_style(linear_layout_system[state.element_index].style);
    }
  });
  if (!tree.container_down()) {
    return false;
  }
  return true;
}

void Gui::container_end() {
  tree.up();
}

void Gui::text(
    const std::string& text,
    const std::function<void(TextStyle&)>& set_style) {
  tree.container_next([&](State& state) {
    state.element_type = ElementType::Text;
    state.element_index = text_system.emplace();
    if (set_style) {
      set_style(text_system[state.element_index].style);
    }
  });

  auto node = tree.current();
  text_system[node->element_index].text = text;
}

Tree::ConstData<std::string> Gui::text_input(
    const std::function<void(TextInputStyle&)>& set_style) {
  tree.container_next([&](State& state) {
    state.element_type = ElementType::TextInput;
    state.element_index = text_input_system.emplace();
    if (set_style) {
      set_style(text_input_system[state.element_index].style);
    }
  });

  auto node = tree.current();
  return tree.data_current<std::string>();
}

void Gui::begin() {
  tree.begin();
}

void Gui::end() {
  tree.end();
  calculate_sizes();
  render();
  event_handling();
}

void Gui::render() {
  window.render_begin();

  {
    std::stack<Tree::ConstPtr> stack;
    stack.push(tree.root());

    while (!stack.empty()) {
      auto node = stack.top();
      stack.pop();

      if (!node.visible() || node->element_index == -1) {
        continue;
      }
      element_system(node->element_type).render(node);

      auto child = node.first_child();
      while (child) {
        stack.push(child);
        child = child.next();
      }
    }
  }

  geometry_renderer.render(window.size());
  text_renderer.render(window.size());
  window.render_end();
}

void Gui::calculate_sizes() {
  {
    struct State {
      Tree::Ptr node;
      bool first_visit;
      State(Tree::Ptr node) : node(node), first_visit(true) {}
    };
    std::stack<State> stack;
    stack.emplace(tree.root());

    while (!stack.empty()) {
      State& state = stack.top();
      auto node = state.node;

      if (!node.visible() || node->element_index == -1) {
        stack.pop();
        continue;
      }

      // If the node has children, process these first
      if (node.first_child() && state.first_visit) {
        state.first_visit = false;
        auto child = node.first_child();
        while (child) {
          stack.emplace(child);
          child = child.next();
        }
        continue;
      }
      stack.pop();

      element_system(node->element_type).set_layout_input(node);
    }
  }

  {
    std::stack<Tree::Ptr> stack;
    stack.emplace(tree.root());
    tree.root()->size = window.size();

    while (!stack.empty()) {
      auto node = stack.top();
      stack.pop();

      if (!node.visible() || node->element_index == -1) {
        continue;
      }

      if (!node.first_child()) {
        continue;
      }

      element_system(node->element_type).set_child_layout_output(node);

      auto child = node.first_child();
      while (child) {
        stack.push(child);
        child = child.next();
      }
    }
  }
}

void Gui::event_handling() {
  window.poll_events();

  for (const auto& event : window.mouse_events()) {
    switch (event.button) {
    case MouseButton::Left:
      event_handling_left_click(event);
      break;
    default:
      break;
    }
  }

  for (const auto& event : window.key_events()) {
    bool handled = false;
    if (event.action == KeyAction::Press) {
      switch (event.key) {
      case Key::Tab:
        focus_next(event.mod_shift);
        handled = true;
        break;
      case Key::Escape:
        focus_escape();
        handled = true;
        break;
      default:
        break;
      }
    }

    if (!handled && node_focus) {
      element_system(node_focus->element_type).key_event(node_focus, event);
    }
  }

  for (const auto& event : window.text_events()) {
    if (node_focus) {
      element_system(node_focus->element_type).text_event(node_focus, event);
    }
  }
}

void Gui::event_handling_left_click(const MouseEvent& event) {
  if (event.action != MouseAction::Press) {
    // Pass-through the hold or release event
    // node_focus should be a valid node, but there may be edge cases where
    // this isn't true (eg: The node gets removed)
    if (node_focus) {
      element_system(node_focus->element_type).mouse_event(node_focus, event);
    }
    return;
  }

  if (node_focus) {
    node_focus->focused = false;
    auto node = node_focus;
    while (node) {
      node->in_focus_tree = false;
      node = node.parent();
    }
  }

  Tree::Ptr prev_node_focus = node_focus;
  node_focus = Tree::Ptr();

  std::stack<Tree::Ptr> stack;
  stack.push(tree.root());

  // TODO: parse floating nodes first when added

  if (!node_focus) {
    // No floating nodes clicked, process standard nodes
    while (!stack.empty()) {
      auto node = stack.top();
      stack.pop();

      Boxf box(node->position, node->position + node->size);
      if (!Boxf(node->position, node->position + node->size)
               .contains(event.position)) {
        continue;
      }
      node->in_focus_tree = true;
      node_focus = node;

      auto child = node.first_child();
      while (child) {
        stack.push(child);
        child = child.next();
      }
    }
  }

  if (node_focus) {
    node_focus->focused = true;
    element_system(node_focus->element_type).mouse_event(node_focus, event);
  }

  if (node_focus != prev_node_focus) {
    if (prev_node_focus) {
      element_system(prev_node_focus->element_type)
          .focus_leave(prev_node_focus, false, node_focus);
    }
    if (node_focus) {
      element_system(node_focus->element_type).focus_enter(node_focus);
    }
  }
}

void Gui::focus_next(bool reverse) {
  printf(
      "Focus next (reverse = %s) - not implemented",
      reverse ? "true" : "false");
}

void Gui::focus_escape() {
  printf("Focus escape - not implemented");
}

void Gui::deinit_node(Tree::ConstPtr node) {
  if (node == node_focus) {
    node_focus = Tree::Ptr();
  }
  element_system(node->element_type).pop(node->element_index);
}

ElementSystem& Gui::element_system(ElementType type) {
  switch (type) {
  case ElementType::LinearLayout:
    return linear_layout_system;
  case ElementType::Text:
    return text_system;
  case ElementType::TextInput:
    return text_input_system;
  case ElementType::Undefined:
    assert(false);
    return linear_layout_system;
  }
  assert(false);
  return linear_layout_system;
}

} // namespace datagui

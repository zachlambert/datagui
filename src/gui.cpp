#include "datagui/gui.hpp"
#include <stack>

namespace datagui {

using namespace std::placeholders;

Gui::Gui(const Window::Config& config) :
    window(config),
    text_renderer(font_manager),
    horizontal_layout_system(geometry_renderer),
    vertical_layout_system(geometry_renderer),
    text_box_system(font_manager, text_renderer),
    text_input_system(font_manager, text_renderer, geometry_renderer),
    button_system(font_manager, geometry_renderer, text_renderer),
    tree(std::bind(&Gui::deinit_node, this, std::placeholders::_1)) {
  geometry_renderer.init();
  text_renderer.init();
}

bool Gui::running() const {
  return window.running();
}

bool Gui::horizontal_layout(
    const std::function<void(HorizontalLayoutStyle&)>& set_style) {
  tree.container_next([&](State& state) {
    state.element_type = ElementType::HorizontalLayout;
    state.element_index = horizontal_layout_system.emplace();
    if (set_style) {
      set_style(horizontal_layout_system[state.element_index].style);
    }
  });
  if (!tree.container_down()) {
    return false;
  }
  return true;
}

bool Gui::vertical_layout(
    const std::function<void(VerticalLayoutStyle&)>& set_style) {
  tree.container_next([&](State& state) {
    state.element_type = ElementType::VerticalLayout;
    state.element_index = vertical_layout_system.emplace();
    if (set_style) {
      set_style(vertical_layout_system[state.element_index].style);
    }
  });
  if (!tree.container_down()) {
    return false;
  }
  return true;
}

void Gui::layout_end() {
  tree.up();
}

void Gui::text_box(
    const std::string& text,
    const std::function<void(TextBoxStyle&)>& set_style) {
  tree.container_next([&](State& state) {
    state.element_type = ElementType::TextBox;
    state.element_index = text_box_system.emplace();
    if (set_style) {
      set_style(text_box_system[state.element_index].style);
    }
  });

  auto node = tree.current();
  text_box_system[node->element_index].text = text;
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

bool Gui::button(
    const std::string& text,
    const std::function<void(ButtonStyle&)>& set_style) {

  tree.container_next([&](State& state) {
    state.element_type = ElementType::Button;
    state.element_index = button_system.emplace();
    if (set_style) {
      set_style(button_system[state.element_index].style);
    }
  });

  auto node = tree.current();
  button_system[node->element_index].text = text;

  auto& element = button_system[node->element_index];
  if (element.released) {
    element.released = false;
    return true;
  }
  return false;
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

      if (debug_mode_) {
        geometry_renderer.queue_box(
            Boxf(node->position, node->position + node->size),
            Color::Clear(),
            2,
            node->focused         ? Color::Blue()
            : node->in_focus_tree ? Color::Red()
                                  : Color::Green(),
            0);

        if (node->focused) {
          std::string debug_text;
          debug_text += "fixed: " + std::to_string(node->fixed_size.x) + ", " +
                        std::to_string(node->fixed_size.y);
          debug_text += "\ndynamic: " + std::to_string(node->dynamic_size.x) +
                        ", " + std::to_string(node->dynamic_size.y);
          debug_text += "\nsize: " + std::to_string(node->size.x) + ", " +
                        std::to_string(node->size.y);

          BoxStyle box_style;
          box_style.bg_color = Color::White();
          box_style.border_width = 2;
          TextStyle text_style;
          text_style.font_size = 24;
          auto text_size = font_manager.text_size(debug_text, text_style);

          geometry_renderer.queue_box(
              Boxf(
                  window.size() - text_size - Vecf::Constant(15),
                  window.size() - Vecf::Constant(5)),
              box_style);
          text_renderer.queue_text(
              window.size() - text_size - Vecf::Constant(10),
              debug_text,
              text_style);
        }
      }

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
        if (node_focus) {
          element_system(node_focus->element_type)
              .focus_leave(node_focus, false, Tree::Ptr());
          set_tree_focus(node_focus, false);
          node_focus = Tree::Ptr();
        }
        handled = true;
        break;
      case Key::D:
        if (event.mod_ctrl) {
          handled = true;
          debug_mode_ = !debug_mode_;
        }
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

  // Clicked -> new focused node

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
    element_system(node_focus->element_type).mouse_event(node_focus, event);
  }

  if (node_focus != prev_node_focus) {
    if (prev_node_focus) {
      element_system(prev_node_focus->element_type)
          .focus_leave(prev_node_focus, true, node_focus);
      set_tree_focus(prev_node_focus, false);
    }
    if (node_focus) {
      element_system(node_focus->element_type).focus_enter(node_focus);
      set_tree_focus(node_focus, true);
    }
  }
}

void Gui::set_tree_focus(Tree::Ptr node, bool value) {
  node->focused = value;
  node->in_focus_tree = value;
  node = node.parent();
  while (node) {
    node->in_focus_tree = value;
    node = node.parent();
  }
}

void Gui::focus_next(bool reverse) {
  if (!tree.root()) {
    return;
  }
  auto next = node_focus;

  do {
    if (!reverse) {
      if (!next) {
        next = tree.root();
      } else if (next.first_child()) {
        next = next.first_child();
      } else if (next.next()) {
        next = next.next();
      } else {
        while (next && !next.next()) {
          next = next.parent();
        }
        if (next) {
          next = next.next();
        }
      }
    } else {
      if (!next) {
        next = tree.root();
        while (next.last_child()) {
          next = next.last_child();
        }
      } else if (!next.prev()) {
        next = next.parent();
      } else {
        next = next.prev();
        while (next.last_child()) {
          next = next.last_child();
        }
      }
    }
  } while (next && next != tree.root() && !next.visible());

  if (next == tree.root() && !tree.root().visible()) {
    next = Tree::Ptr();
  }

  if (node_focus) {
    auto prev_node_focus = node_focus;
    element_system(prev_node_focus->element_type)
        .focus_leave(prev_node_focus, true, next);
    set_tree_focus(node_focus, false);
  }

  node_focus = next;

  if (node_focus) {
    set_tree_focus(node_focus, true);
    element_system(node_focus->element_type).focus_enter(node_focus);
  }
}

void Gui::deinit_node(Tree::ConstPtr node) {
  if (node == node_focus) {
    element_system(node_focus->element_type)
        .focus_leave(node_focus, false, Tree::Ptr());
    set_tree_focus(node_focus, false);
    node_focus = Tree::Ptr();
  }
  element_system(node->element_type).pop(node->element_index);
}

ElementSystem& Gui::element_system(ElementType type) {
  switch (type) {
  case ElementType::HorizontalLayout:
    return horizontal_layout_system;
  case ElementType::VerticalLayout:
    return vertical_layout_system;
  case ElementType::TextBox:
    return text_box_system;
  case ElementType::TextInput:
    return text_input_system;
  case ElementType::Button:
    return button_system;
  case ElementType::Undefined:
    assert(false);
    return horizontal_layout_system;
  }
  assert(false);
  return horizontal_layout_system;
}

} // namespace datagui

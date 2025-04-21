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
    button_system(font_manager, geometry_renderer, text_renderer) {

  horizontal_layout_system.register_type(tree, systems);
  vertical_layout_system.register_type(tree, systems);
  text_box_system.register_type(tree, systems);
  text_input_system.register_type(tree, systems);
  button_system.register_type(tree, systems);

  geometry_renderer.init();
  text_renderer.init();
}

bool Gui::running() const {
  return window.running();
}

bool Gui::horizontal_layout(const SetHorizontalLayoutStyle& set_style) {
  auto element = tree.next();
  horizontal_layout_system.visit(element, set_style);
  return tree.down_if();
}

bool Gui::vertical_layout(
    const std::function<void(VerticalLayoutStyle&)>& set_style) {
  auto element = tree.next(vertical_layout_system.type());
  vertical_layout_system.visit(element, set_style);
  return tree.down_if();
}

void Gui::layout_end() {
  tree.up();
}

void Gui::text_box(
    const std::string& text,
    const std::function<void(TextBoxStyle&)>& set_style) {
  auto element = tree.next(text_box_system.type());
  text_box_system.visit(element, text, set_style);
}

const std::string* Gui::text_input(
    const std::string& initial_value,
    const std::function<void(TextInputStyle&)>& set_style) {
  auto element = tree.next(text_input_system.type());
  return text_input_system.visit(element, initial_value, set_style);
}

void Gui::text_input(
    const Variable<std::string>& variable,
    const std::function<void(TextInputStyle&)>& set_style) {
  auto element = tree.next(text_input_system.type());
  text_input_system.visit(element, variable, set_style);
}

bool Gui::button(
    const std::string& text,
    const std::function<void(ButtonStyle&)>& set_style) {
  auto element = tree.next(button_system.type());
  return button_system.visit(element, text, set_style);
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
    std::stack<ConstElement> stack;
    stack.push(tree.root());

    while (!stack.empty()) {
      auto element = stack.top();
      stack.pop();

      if (!element.visible()) {
        continue;
      }
      systems.render(element);

      if (debug_mode_) {
        geometry_renderer.queue_box(
            Boxf(element->position, element->position + element->size),
            Color::Clear(),
            2,
            element->focused         ? Color::Blue()
            : element->in_focus_tree ? Color::Red()
                                     : Color::Green(),
            0);

        if (element->focused) {
          std::string debug_text;
          debug_text += element.debug();
          debug_text += "\nfixed: " + std::to_string(element->fixed_size.x) +
                        ", " + std::to_string(element->fixed_size.y);
          debug_text +=
              "\ndynamic: " + std::to_string(element->dynamic_size.x) + ", " +
              std::to_string(element->dynamic_size.y);
          debug_text += "\nsize: " + std::to_string(element->size.x) + ", " +
                        std::to_string(element->size.y);

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

      auto child = element.first_child();
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
      Element element;
      bool first_visit;
      State(Element element) : element(element), first_visit(true) {}
    };
    std::stack<State> stack;
    stack.emplace(tree.root());

    while (!stack.empty()) {
      State& state = stack.top();
      auto element = state.element;

      if (!element.visible()) {
        stack.pop();
        continue;
      }

      // If the node has children, process these first
      if (element.first_child() && state.first_visit) {
        state.first_visit = false;
        auto child = element.first_child();
        while (child) {
          stack.emplace(child);
          child = child.next();
        }
        continue;
      }
      stack.pop();

      systems.set_layout_input(element);
    }
  }

  {
    std::stack<Element> stack;
    stack.emplace(tree.root());
    tree.root()->size = window.size();

    while (!stack.empty()) {
      auto element = stack.top();
      stack.pop();

      if (!element.visible()) {
        continue;
      }

      if (!element.first_child()) {
        continue;
      }

      element_systems.at(element.type())->set_child_layout_output(element);

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
        if (element_focus) {
          systems.focus_leave(element_focus, false, Element());
          set_tree_focus(element_focus, false);
          element_focus = Element();
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

    if (!handled && element_focus) {
      systems.key_event(element_focus, event);
    }
  }

  for (const auto& event : window.text_events()) {
    if (element_focus) {
      systems.text_event(element_focus, event);
    }
  }
}

void Gui::event_handling_left_click(const MouseEvent& event) {
  if (event.action != MouseAction::Press) {
    // Pass-through the hold or release event
    // node_focus should be a valid node, but there may be edge cases where
    // this isn't true (eg: The node gets removed)
    if (element_focus) {
      systems.mouse_event(element_focus, event);
    }
    return;
  }

  // Clicked -> new focused node

  Element prev_element_focus = element_focus;
  element_focus = Element();

  std::stack<Element> stack;
  stack.push(tree.root());

  // TODO: parse floating nodes first when added

  if (!element_focus) {
    // No floating nodes clicked, process standard nodes
    while (!stack.empty()) {
      auto element = stack.top();
      stack.pop();

      if (!element->box().contains(event.position)) {
        continue;
      }
      element->in_focus_tree = true;
      element_focus = element;

      auto child = element.first_child();
      while (child) {
        stack.push(child);
        child = child.next();
      }
    }
  }

  if (element_focus != prev_element_focus) {
    if (prev_element_focus) {
      systems.focus_leave(prev_element_focus, true, element_focus);
      set_tree_focus(prev_element_focus, false);
    }
    if (element_focus) {
      // Only use focus_enter() for non-click focus enter (ie: tab into the
      // element)
      set_tree_focus(element_focus, true);
    }
  }
  if (element_focus) {
    systems.mouse_event(element_focus, event);
  }
}

void Gui::set_tree_focus(Element element, bool value) {
  element->focused = value;
  element->in_focus_tree = value;
  element = element.parent();
  while (element) {
    element->in_focus_tree = value;
    element = element.parent();
  }
}

void Gui::focus_next(bool reverse) {
  if (!tree.root()) {
    return;
  }
  auto next = element_focus;

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
    next = Element();
  }

  if (element_focus) {
    auto prev_element_focus = element_focus;
    systems.focus_leave(prev_element_focus, true, next);
    set_tree_focus(element_focus, false);
  }

  element_focus = next;

  if (element_focus) {
    set_tree_focus(element_focus, true);
    systems.focus_enter(element_focus);
  }
}

} // namespace datagui

#include "datagui/gui.hpp"
#include <sstream>
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
    drop_down_system(font_manager, text_renderer, geometry_renderer) {

  horizontal_layout_system.register_type(tree, systems);
  vertical_layout_system.register_type(tree, systems);
  text_box_system.register_type(tree, systems);
  text_input_system.register_type(tree, systems);
  button_system.register_type(tree, systems);
  drop_down_system.register_type(tree, systems);

  geometry_renderer.init();
  text_renderer.init();
}

bool Gui::running() const {
  return window.running();
}

bool Gui::horizontal_layout(const SetHorizontalLayoutStyle& set_style) {
  auto element = tree.next(horizontal_layout_system.type());
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
    const std::string& initial_text,
    const std::function<void(TextInputStyle&)>& set_style) {
  auto element = tree.next(text_input_system.type());
  return text_input_system.visit(element, initial_text, set_style);
}

void Gui::text_input(
    const Variable<std::string>& text,
    const std::function<void(TextInputStyle&)>& set_style) {
  auto element = tree.next(text_input_system.type());
  text_input_system.visit(element, text, set_style);
}

bool Gui::button(
    const std::string& text,
    const std::function<void(ButtonStyle&)>& set_style) {
  auto element = tree.next(button_system.type());
  return button_system.visit(element, text, set_style);
}

const int* Gui::drop_down(
    const std::vector<std::string>& choices,
    int initial_choice,
    const SetDropDownStyle& set_style) {
  auto element = tree.next(drop_down_system.type());
  return drop_down_system.visit(element, choices, initial_choice, set_style);
}

void Gui::drop_down(
    const std::vector<std::string>& choices,
    const Variable<int>& choice,
    const SetDropDownStyle& set_style) {
  auto element = tree.next(drop_down_system.type());
  drop_down_system.visit(element, choices, choice, set_style);
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
    struct State {
      ConstElement element;
      bool first_visit;
      State(ConstElement element) : element(element), first_visit(true) {}
    };
    std::stack<State> stack;
    stack.emplace(tree.root());

    while (!stack.empty()) {
      auto& state = stack.top();
      const auto& element = state.element;

      if (!state.first_visit) {
        geometry_renderer.pop_mask();
        stack.pop();
        continue;
      }
      state.first_visit = false;

      if (!element.visible()) {
        continue;
      }
      systems.render(element);

      if (debug_mode_) {
        geometry_renderer.queue_box(
            Boxf(element->position, element->position + element->size),
            1,
            Color::Clear(),
            2,
            element->focused         ? Color::Blue()
            : element->in_focus_tree ? Color::Red()
                                     : Color::Green(),
            0);

        if (element->focused) {
          geometry_renderer.queue_box(
              element->hitbox,
              1,
              Color::Clear(),
              2,
              Color(1, 0, 1),
              0);
        }

        if (element->focused) {
          std::stringstream ss;

          ss << element.debug();
          ss << "\nfixed: " << element->fixed_size.x << ", "
             << element->fixed_size.y;
          ss << "\ndynamic: " << element->dynamic_size.x << ", "
             << element->dynamic_size.y;
          ss << "\nsize: " << element->size.x << ", " << element->size.y;
          ss << "\nz_range: " << element->z_range.lower << ", "
             << element->z_range.upper;
          std::string debug_text = ss.str();

          BoxStyle box_style;
          box_style.bg_color = Color::White();
          box_style.border_width = 2;
          TextStyle text_style;
          text_style.font_size = 24;
          auto text_size =
              font_manager.text_size(debug_text, text_style, LengthWrap());

          geometry_renderer.queue_box(
              Boxf(
                  window.size() - text_size - Vecf::Constant(15),
                  window.size() - Vecf::Constant(5)),
              1,
              box_style);
          text_renderer.queue_text(
              window.size() - text_size - Vecf::Constant(10),
              1,
              debug_text,
              text_style,
              LengthWrap());
        }
      }

      geometry_renderer.push_mask(element->box());

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
    tree.root()->position = Vecf::Zero();
    tree.root()->size = window.size();
    tree.root()->z_range = Rangef(0, 1);

    while (!stack.empty()) {
      auto element = stack.top();
      stack.pop();

      if (!element.visible()) {
        continue;
      }

      if (!element.first_child()) {
        continue;
      }

      systems.set_child_layout_output(element);

      auto child = element.first_child();
      while (child) {
        stack.push(child);
        child = child.next();
      }
    }
  }

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
      if (state.first_visit && element.first_child()) {
        state.first_visit = false;
        auto child = element.first_child();
        while (child) {
          stack.emplace(child);
          child = child.next();
        }
        continue;
      }
      stack.pop();

      element->hitbox.lower = element->position + element->hitbox_offset.lower;
      element->hitbox.upper =
          element->position + element->size + element->hitbox_offset.upper;

      element->bounding_box = element->hitbox;
      auto child = element.first_child();
      while (child) {
        element->bounding_box = bounding(element->bounding_box, child->hitbox);
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

  for (const auto& event : window.scroll_events()) {
    event_handling_scroll(event);
  }

  event_handling_hover(window.mouse_pos());

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

Element Gui::get_leaf_node(const Vecf& position) {
  Element leaf = Element();
  float leaf_z = 0;

  std::stack<Element> stack;
  stack.push(tree.root());

  while (!stack.empty()) {
    auto element = stack.top();
    stack.pop();

    if (!element->bounding_box.contains(position)) {
      continue;
    }
    if (element->z_range.lower < leaf_z) {
      continue;
    }

    // The bounding box for a node can be larger than the hitbox
    // for this specific node (eg: a child hitbox extends outside the
    // parent hitbox)
    if (element->hitbox.contains(position)) {
      leaf = element;
      leaf_z = element->z_range.lower;
    }

    auto child = element.first_child();
    while (child) {
      stack.push(child);
      child = child.next();
    }
  }

  return leaf;
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
  element_focus = get_leaf_node(event.position);

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

void Gui::event_handling_hover(const Vecf& mouse_pos) {
  if (element_hover) {
    element_hover->hovered = false;
  }

  element_hover = get_leaf_node(mouse_pos);
  if (!element_hover) {
    return;
  }
  element_hover->hovered = true;
  systems.mouse_hover(element_hover, mouse_pos);
}

void Gui::event_handling_scroll(const ScrollEvent& event) {
  Element element = get_leaf_node(event.position);
  while (element) {
    if (systems.scroll_event(element, event)) {
      return;
    }
    element = element.parent();
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

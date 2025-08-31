#include "datagui/gui.hpp"
#include "datagui/log.hpp"
#include <queue>
#include <sstream>
#include <stack>

#include "datagui/element/button.hpp"
#include "datagui/element/checkbox.hpp"
#include "datagui/element/dropdown.hpp"
#include "datagui/element/floating.hpp"
#include "datagui/element/series.hpp"
#include "datagui/element/text_box.hpp"
#include "datagui/element/text_input.hpp"

#define BOOL_STR(value) (value ? "true" : "false")

namespace datagui {

using namespace std::placeholders;

Gui::Gui(const Window::Config& config) : window(config) {
  fm = std::make_shared<FontManager>();
  sm = std::make_shared<StyleManager>();
  renderer.init(fm);

  systems.emplace<ButtonSystem>(fm);
  systems.emplace<CheckboxSystem>(fm);
  systems.emplace<DropdownSystem>(fm);
  systems.emplace<FloatingSystem>(fm);
  systems.emplace<SeriesSystem>();
  systems.emplace<TextBoxSystem>(fm);
  systems.emplace<TextInputSystem>(fm);
}

bool Gui::running() const {
  return window.running();
}

bool Gui::series_begin() {
  auto element = tree.next();
  auto& props = get_series(systems, *element);

  props.set_style(*sm);

  if (tree.down_if()) {
    DATAGUI_LOG("Gui::series_begin", "DOWN");
    sm->down();
    return true;
  }
  DATAGUI_LOG("Gui::series_begin", "SKIP");
  return false;
}

void Gui::series_end() {
  sm->up();
  tree.up();
}

void Gui::text_box(const std::string& text) {
  auto element = tree.next();
  auto& props = get_text_box(systems, *element);

  props.set_style(*sm);
  props.text = text;
}

bool Gui::button(const std::string& text) {
  auto element = tree.next();
  auto& props = get_button(systems, *element);

  props.text = text;
  props.set_style(*sm);
  if (props.released) {
    DATAGUI_LOG("Gui::button", "Button released");
    props.released = false;
    return true;
  }
  return false;
}

const std::string* Gui::text_input(const std::string& initial_value) {
  auto element = tree.next();
  auto& props = get_text_input(systems, *element, initial_value);

  props.set_style(*sm);

  if (props.changed) {
    DATAGUI_LOG("Gui::text_input(initial_value)", "Text input changed");
    props.changed = false;
    return &props.text;
  }
  return nullptr;
}

void Gui::text_input(const Variable<std::string>& value) {
  auto element = tree.next();
  auto& props = get_text_input(systems, *element, *value);

  props.set_style(*sm);

  if (props.changed) {
    DATAGUI_LOG("Gui::text_input(variable)", "Text input changed");
    value.set_internal(props.text);
    props.changed = false;
  } else if (value.modified_external()) {
    DATAGUI_LOG("Gui::text_input(variable)", "Variable modified");
    props.text = *value;
  }
}

const bool* Gui::checkbox(bool initial_value) {
  auto element = tree.next();
  auto& props = get_checkbox(systems, *element, initial_value);

  props.set_style(*sm);

  if (props.changed) {
    DATAGUI_LOG(
        "Gui::checkbox(initial_value)",
        "Checkbox changed -> %s",
        props.checked ? "true" : "false");
    props.changed = false;
    return &props.checked;
  }
  return nullptr;
}

void Gui::checkbox(const Variable<bool>& value) {
  auto element = tree.next();
  auto& props = get_checkbox(systems, *element, *value);

  props.set_style(*sm);

  if (props.changed) {
    DATAGUI_LOG(
        "Gui::checkbox(variable)",
        "Checkbox changed -> %s",
        BOOL_STR(props.checked));
    value.set_internal(props.checked);
    props.changed = false;
  } else if (value.modified_external()) {
    DATAGUI_LOG(
        "Gui::checkbox(variable)",
        "Variable modified -> %s",
        BOOL_STR(*value));
    props.checked = *value;
  }
}

const int* Gui::dropdown(
    const std::vector<std::string>& choices,
    int initial_choice) {
  auto element = tree.next();
  auto& props = get_dropdown(systems, *element, choices, initial_choice);

  props.set_style(*sm);

  if (props.changed) {
    DATAGUI_LOG(
        "Gui::dropdown(initial_value)",
        "Dropdown changed -> %i",
        props.choice);
    props.changed = false;
    return &props.choice;
  }
  return nullptr;
}

void Gui::dropdown(
    const std::vector<std::string>& choices,
    const Variable<int>& choice) {
  auto element = tree.next();
  auto& props = get_dropdown(systems, *element, choices, *choice);

  props.set_style(*sm);

  if (props.changed) {
    DATAGUI_LOG(
        "Gui::dropdown(variable)",
        "Dropdown changed -> %i",
        props.choice);
    choice.set_internal(props.choice);
    props.changed = false;
  } else if (choice.modified_external()) {
    DATAGUI_LOG("Gui::dropdown(variable)", "Variable modified -> %i", *choice);
    props.choice = *choice;
  }
}

bool Gui::floating_begin(const Variable<bool>& open, const std::string& title) {
  auto element = tree.next();
  auto props = get_floating(systems, *element, *open);

  props.title = title;
  props.set_style(*sm);

  if (props.open_changed) {
    DATAGUI_LOG(
        "Gui::floating_begin",
        "Open changed -> %s",
        BOOL_STR(props.open));
    props.open_changed = false;
    open.set_internal(props.open);
  } else if (open.modified_external()) {
    DATAGUI_LOG(
        "Gui::floating_begin",
        "Open variable changed -> %s",
        BOOL_STR(*open));
    props.open = *open;
  }

  if (!*open) {
    return false;
  }
  if (tree.down_if()) {
    sm->down();
    return true;
  }
  return false;
}

void Gui::floating_end() {
  sm->up();
  tree.up();
}

bool Gui::begin() {
  return tree.begin();
}

void Gui::end() {
  tree.end();
}

void Gui::poll() {
  // TODO: Can some of this be put in end() ?
  calculate_sizes();
  render();
  event_handling();
}

void Gui::render() {
  window.render_begin();

  struct State {
    ConstElementPtr element;
    bool first_visit;
    State(ConstElementPtr element) : element(element), first_visit(true) {}
  };
  std::stack<State> layer_stack;
  layer_stack.emplace(tree.root());

  struct Compare {
    bool operator()(const ConstElementPtr& lhs, const ConstElementPtr& rhs) {
      return lhs->layer >= rhs->layer;
    }
  };
  std::priority_queue<ConstElementPtr, std::vector<ConstElementPtr>, Compare>
      queued_elements;
  int current_layer = layer_stack.top().element->layer;

  while (true) {
    if (layer_stack.empty()) {
      renderer.render(window.size());
      renderer.render(window.size());
      if (queued_elements.empty()) {
        break;
      }
      current_layer = queued_elements.top()->layer;
      layer_stack.emplace(queued_elements.top());
      queued_elements.pop();
    }
    auto& state = layer_stack.top();
    const auto& element = state.element;

    if (!state.first_visit) {
#if 0
      renderer.pop_mask();
#endif
      layer_stack.pop();
      continue;
    }
    state.first_visit = false;

    if (element->hidden) {
      continue;
    }
    systems.get(*element).render(*element, renderer);

#ifdef DATAGUI_DEBUG
    if (debug_mode_) {
      renderer.queue_box(
          Boxf(element->position, element->position + element->size),
          Color::Clear(),
          2,
          element->focused         ? Color::Blue()
          : element->in_focus_tree ? Color::Red()
                                   : Color::Green(),
          0);

      if (element->floating) {
        renderer.queue_box(
            element->float_box,
            Color::Clear(),
            2,
            element->in_focus_tree ? Color(1, 0, 1) : Color(0, 1, 1),
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
        ss << "\nlayer: " << element->layer;
        std::string debug_text = ss.str();

        TextStyle text_style;
        text_style.font_size = 24;
        auto text_size = fm->text_size(debug_text, text_style, LengthWrap());

        renderer.queue_box(
            Boxf(
                window.size() - text_size - Vecf::Constant(15),
                window.size() - Vecf::Constant(5)),
            Color::White(),
            2,
            Color::Black(),
            0);
        renderer.queue_text(
            window.size() - text_size - Vecf::Constant(10),
            debug_text,
            text_style,
            LengthWrap());
      }
    }
#endif

#if 0
    if (element->floating) {
      res.geometry_renderer.push_mask(element->float_box);
      res.text_renderer.push_mask(element->float_box);
    } else {
      res.geometry_renderer.push_mask(element->box());
      res.text_renderer.push_mask(element->box());
    }
#endif

    for (auto child = element.first_child(); child; child = child.next()) {
      if (child->layer == current_layer) {
        layer_stack.push(child);
      } else {
        queued_elements.push(child);
      }
    }
  }

  window.render_end();
}

void Gui::calculate_sizes() {
  {
    struct State {
      ElementPtr element;
      bool first_visit;
      State(ElementPtr element) : element(element), first_visit(true) {}
    };

    std::stack<State> stack;
    stack.emplace(tree.root());

    while (!stack.empty()) {
      State& state = stack.top();
      auto element = state.element;

      if (element->hidden) {
        stack.pop();
        continue;
      }

      // If the node has children, process these first
      if (element.first_child() && state.first_visit) {
        state.first_visit = false;
        for (auto child = element.first_child(); child; child = child.next()) {
          stack.emplace(child);
        }
        continue;
      }
      stack.pop();

      std::vector<const Element*> children;
      for (auto child = element.first_child(); child; child = child.next()) {
        children.push_back(child.get());
      }
      systems.get(*element).set_input_state(*element, children);
    }
  }

  {
    struct Compare {
      bool operator()(const ElementPtr& lhs, const ElementPtr& rhs) const {
        return lhs->float_priority > rhs->float_priority;
      }
    };
    struct Layer {
      ConstElementPtr root;
      std::priority_queue<ElementPtr, std::vector<ElementPtr>, Compare>
          floating;
      Layer(ConstElementPtr root) : root(root) {}
    };

    std::stack<Layer> layers;
    std::stack<ElementPtr> stack;
    {
      auto root = tree.root();
      root->position = Vecf::Zero();
      root->size = window.size();
      root->layer_box = root->box();
      root->layer = 0;
      stack.push(root);
      layers.emplace(root);
    }
    int next_layer = 1;

    while (true) {
      if (stack.empty()) {
        if (layers.empty()) {
          break;
        }
        Layer& layer = layers.top();
        if (layer.floating.empty()) {
          layers.pop();
          continue;
        }

        ElementPtr next_floating = layer.floating.top();
        next_floating->layer_box = layer.root->box();
        next_floating->layer = next_layer;
        next_layer++;
        stack.push(next_floating);
        layers.emplace(next_floating);

        layer.floating.pop();
      }

      auto element = stack.top();
      stack.pop();

      if (element->hidden) {
        continue;
      }

      if (element->floating && element != layers.top().root) {
        // TODO: Handle this
#if 0
        if (element.is_new()) {
          element->float_priority = next_float_priority++;
        }
#endif
        layers.top().floating.push(element);
        continue;
      }

      std::vector<Element*> children;
      for (auto child = element.first_child(); child; child = child.next()) {
        children.push_back(child.get());
      }
      systems.get(*element).set_dependent_state(*element, children);

      for (auto child = element.first_child(); child; child = child.next()) {
        child->layer_box = element->layer_box;
        child->layer = element->layer;
        stack.push(child);
      }
    }
  }

  {
    struct State {
      ElementPtr element;
      bool first_visit;
      State(ElementPtr element) : element(element), first_visit(true) {}
    };

    std::stack<State> stack;
    stack.emplace(tree.root());

    while (!stack.empty()) {
      State& state = stack.top();
      auto element = state.element;

      if (element->hidden) {
        stack.pop();
        continue;
      }

      // If the node has children, process these first
      if (element.first_child() && state.first_visit) {
        state.first_visit = false;
        for (auto child = element.first_child(); child; child = child.next()) {
          stack.emplace(child);
        }
        continue;
      }
      stack.pop();

      if (element->floating) {
        element->hitbox = element->float_box;
      } else {
        element->hitbox = element->box();
      }
      element->hitbox_bounds = element->hitbox;
      for (auto child = element.first_child(); child; child = child.next()) {
        element->hitbox_bounds =
            bounding(element->hitbox_bounds, child->hitbox);
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
          element_focus.revisit(
              systems.get(*element_focus).focus_leave(*element_focus, false));
          set_tree_focus(element_focus, false);
          element_focus = ElementPtr();
        }
        handled = true;
        break;
#ifdef DATAGUI_DEBUG
      case Key::D:
        if (event.mod_ctrl) {
          handled = true;
          debug_mode_ = !debug_mode_;
        }
#endif
      default:
        break;
      }
    }

    if (!handled && element_focus) {
      element_focus.revisit(
          systems.get(*element_focus).key_event(*element_focus, event));
    }
  }

  for (const auto& event : window.text_events()) {
    if (element_focus) {
      element_focus.revisit(
          systems.get(*element_focus).text_event(*element_focus, event));
    }
  }
}

ElementPtr Gui::get_leaf_node(const Vecf& position) {
  ElementPtr leaf = ElementPtr();
  int leaf_layer = -1;

  std::stack<ElementPtr> stack;
  stack.push(tree.root());

  while (!stack.empty()) {
    auto element = stack.top();
    stack.pop();

    if (!element->hitbox_bounds.contains(position)) {
      continue;
    }

    if (element->hitbox.contains(position) && element->layer >= leaf_layer) {
      leaf = element;
      leaf_layer = element->layer;
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
      element_focus.revisit(
          systems.get(*element_focus).mouse_event(*element_focus, event));
    }
    return;
  }

  // Clicked -> new focused node

  ElementPtr prev_element_focus = element_focus;
  element_focus = get_leaf_node(event.position);

  if (element_focus != prev_element_focus) {
    if (prev_element_focus) {
      prev_element_focus.revisit(systems.get(*prev_element_focus)
                                     .focus_leave(*prev_element_focus, true));
      set_tree_focus(prev_element_focus, false);
    }
    if (element_focus) {
      // Only use focus_enter() for non-click focus enter (ie: tab into the
      // element)
      set_tree_focus(element_focus, true);
    }
  }
  if (element_focus) {
    element_focus.revisit(
        systems.get(*element_focus).mouse_event(*element_focus, event));
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
  systems.get(*element_hover).mouse_hover(*element_hover, mouse_pos);
}

void Gui::event_handling_scroll(const ScrollEvent& event) {
  ElementPtr element = get_leaf_node(event.position);
  while (element) {
    if (systems.get(*element).scroll_event(*element, event)) {
      return;
    }
    element = element.parent();
  }
}

void Gui::set_tree_focus(ElementPtr element, bool focused) {
  element->focused = focused;
  element->in_focus_tree = focused;
  if (focused) {
    element->float_priority = next_float_priority++;
  }

  element = element.parent();
  while (element) {
    element->in_focus_tree = focused;
    if (focused) {
      element->float_priority = next_float_priority++;
    }
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
  } while (next && next != tree.root() && next->hidden);

  if (next == tree.root() && tree.root()->hidden) {
    next = ElementPtr();
  }

  if (element_focus) {
    auto prev_element_focus = element_focus;
    prev_element_focus.revisit(systems.get(*prev_element_focus)
                                   .focus_leave(*prev_element_focus, true));
    set_tree_focus(element_focus, false);
  }

  element_focus = next;

  if (element_focus) {
    set_tree_focus(element_focus, true);
    systems.get(*element_focus).focus_enter(*element_focus);
  }
}

} // namespace datagui

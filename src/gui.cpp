#include "datagui/gui.hpp"
#include "datagui/log.hpp"
#include <queue>
#include <sstream>
#include <stack>

#include "datagui/system/button.hpp"
#include "datagui/system/checkbox.hpp"
#include "datagui/system/dropdown.hpp"
#include "datagui/system/floating.hpp"
#include "datagui/system/labelled.hpp"
#include "datagui/system/section.hpp"
#include "datagui/system/series.hpp"
#include "datagui/system/text_box.hpp"
#include "datagui/system/text_input.hpp"

#define BOOL_STR(value) (value ? "true" : "false")

namespace datagui {

using namespace std::placeholders;

Gui::Gui(const Window::Config& config) : window(config) {
  fm = std::make_shared<FontManager>();
  theme = std::make_shared<Theme>(theme_default());
  renderer.init(fm);

  systems.resize(TypeCount);
  systems[(std::size_t)Type::Button] =
      std::make_unique<ButtonSystem>(fm, theme);
  systems[(std::size_t)Type::Checkbox] =
      std::make_unique<CheckboxSystem>(fm, theme);
  systems[(std::size_t)Type::Dropdown] =
      std::make_unique<DropdownSystem>(fm, theme);
  systems[(std::size_t)Type::Floating] =
      std::make_unique<FloatingSystem>(fm, theme);
  systems[(std::size_t)Type::Labelled] =
      std::make_unique<LabelledSystem>(fm, theme);
  systems[(std::size_t)Type::Section] =
      std::make_unique<SectionSystem>(fm, theme);
  systems[(std::size_t)Type::Series] = std::make_unique<SeriesSystem>(theme);
  systems[(std::size_t)Type::TextBox] =
      std::make_unique<TextBoxSystem>(fm, theme);
  systems[(std::size_t)Type::TextInput] =
      std::make_unique<TextInputSystem>(fm, theme);
}

bool Gui::running() const {
  return window.running();
}

bool Gui::series_begin() {
  current.force(Type::Series);
  args_series_.apply(current.series());
  args_series_.reset();

  if (current.dirty()) {
    stack.push(current);
    current = current.child();
    return true;
  }
  return false;
}

void Gui::series_end() {
  current = stack.top();
  stack.pop();
}

#if 0
bool Gui::labelled_begin(const std::string& label) {
  auto element = tree.next();
  auto& props = get_labelled(systems, *element, label);

  if (tree.down_if()) {
    DATAGUI_LOG("Gui::labelled_begin", "DOWN");
    return true;
  }
  DATAGUI_LOG("Gui::labelled_begin", "SKIP");
  return false;
}

void Gui::labelled_end() {
  tree.up();
}

bool Gui::section_begin(const std::string& label) {
  auto element = tree.next();
  auto& props = get_section(systems, *element, label);

  if (tree.down_if()) {
    DATAGUI_LOG("Gui::section_begin", "DOWN");
    return true;
  }
  DATAGUI_LOG("Gui::section_begin", "SKIP");
  return false;
}

void Gui::section_end() {
  tree.up();
}
#endif

void Gui::text_box(const std::string& text) {
  current.force(Type::TextBox);
  auto& text_box = current.text_box();
  text_box.text = text;
  current = current.next();
}

#if 0
bool Gui::button(const std::string& text) {
  auto element = tree.next();
  auto& props = get_button(systems, *element);

  props.text = text;
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

bool Gui::floating_begin(
    const Variable<bool>& open,
    const std::string& title,
    float width,
    float height) {

  auto element = tree.next();
  auto& props = get_floating(systems, *element, *open);
  args_floating_.apply(props);
  args_floating_.reset();

  props.title = title;
  props.width = width;
  props.height = height;

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

  if (!props.open && !element->hidden) {
    tree.down();
    tree.up();
  }
  element->hidden = !props.open;

  if (!props.open) {
    return false;
  }
  return tree.down_if();
}

void Gui::floating_end() {
  tree.up();
}

#endif

bool Gui::begin() {
  current = tree.root();
  return current.dirty();
}

void Gui::end() {
  assert(stack.empty());
}

void Gui::poll() {
  calculate_sizes();
  render();
  event_handling();
}

void Gui::render() {
  auto render_tree = [this](ConstElementPtr root) {
    struct State {
      ConstElementPtr element;
      bool first_visit;
      State(ConstElementPtr element) : element(element), first_visit(true) {}
    };
    std::stack<State> stack;
    stack.push(root);

    while (!stack.empty()) {
      auto& state = stack.top();
      const auto& element = state.element;

      if (element.state().hidden ||
          (element != root && element.state().floating)) {
        stack.pop();
        continue;
      }

      if (!state.first_visit) {
        renderer.pop_mask();
        stack.pop();
        continue;
      }
      state.first_visit = false;

      system(element).render(element, renderer);
      renderer.push_mask(
          element.state().floating ? element.state().float_box
                                   : element.state().box());

      for (auto child = element.child(); child.exists(); child = child.next()) {
        stack.push(child);
      }
    }
  };

  window.render_begin();
  renderer.render_begin(window.size());

  render_tree(tree.root());
  renderer.render();

  for (auto element : floating_elements) {
    render_tree(element);
    renderer.render();
  }
#ifdef DATAGUI_DEBUG
  if (debug_mode_) {
    debug_render();
    renderer.render();
  }
#endif

  renderer.render_end();
  window.render_end();
}

#ifdef DATAGUI_DEBUG
void Gui::debug_render() {
  struct State {
    ConstElementPtr element;
    bool first_visit;
    State(ConstElementPtr element) : element(element), first_visit(true) {}
  };
  std::stack<State> layer_stack;
  layer_stack.emplace(tree.root());

  ConstElementPtr focused;

  while (!layer_stack.empty()) {
    auto& state = layer_stack.top();
    auto element = state.element;

    if (element.state().hidden) {
      layer_stack.pop();
      continue;
    }
    if (!state.first_visit) {
      renderer.pop_mask();
      layer_stack.pop();
      continue;
    }
    state.first_visit = false;

    renderer.queue_box(
        Boxf(
            element.state().position,
            element.state().position + element.state().size),
        Color::Clear(),
        2,
        element.state().focused         ? Color::Blue()
        : element.state().in_focus_tree ? Color::Red()
                                        : Color::Green(),
        0);

    if (element.state().floating) {
      renderer.queue_box(
          element.state().float_box,
          Color::Clear(),
          2,
          element.state().in_focus_tree ? Color(1, 0, 1) : Color(0, 1, 1),
          0);
    }

    if (element.state().floating) {
      renderer.push_mask(element.state().float_box);
    } else {
      renderer.push_mask(element.state().box());
    }

    for (auto child = element.child(); child.exists(); child = child.next()) {
      layer_stack.push(child);
    }

    if (element.state().focused) {
      focused = element;
    }
  }

  if (focused.exists()) {
    std::stringstream ss;

    ss << "fixed: " << focused.state().fixed_size.x << ", "
       << focused.state().fixed_size.y;
    ss << "\ndynamic: " << focused.state().dynamic_size.x << ", "
       << focused.state().dynamic_size.y;
    ss << "\nsize: " << focused.state().size.x << ", "
       << focused.state().size.y;
    if (focused.state().floating) {
      ss << "\nfloating priority: " << focused.state().float_priority;
    }
    std::string debug_text = ss.str();

    auto text_size =
        fm->text_size(debug_text, Font::DejaVuSans, 24, LengthWrap());

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
        Font::DejaVuSans,
        24,
        Color::Black(),
        LengthWrap());
  }
}
#endif

void Gui::calculate_sizes() {
  if (!tree.root().exists()) {
    return;
  }

  floating_elements.clear();
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

      if (element.state().hidden) {
        stack.pop();
        continue;
      }

      // If the node has children, process these first
      if (element.child().exists() && state.first_visit) {
        state.first_visit = false;
        for (auto child = element.child(); child.exists();
             child = child.next()) {
          stack.emplace(child);
        }
        continue;
      }
      stack.pop();

      system(element).set_input_state(element);
    }
  }

  {
    std::stack<ElementPtr> stack;
    {
      auto root = tree.root();
      assert(root.exists());
      root.state().position = Vecf::Zero();
      root.state().size = window.size();
      stack.push(root);
    }

    while (!stack.empty()) {
      auto element = stack.top();
      stack.pop();

      if (element.state().hidden) {
        continue;
      }

      if (element.state().floating) {
        floating_elements.insert(element);
        if (auto type = std::get_if<FloatingTypeAbsolute>(
                &element.state().floating_type)) {
          element.state().float_box.lower =
              window.size() / 2.f - type->size / 2.f;
          element.state().float_box.upper =
              window.size() / 2.f + type->size / 2.f;
        } else if (
            auto type = std::get_if<FloatingTypeRelative>(
                &element.state().floating_type)) {
          element.state().float_box.lower =
              element.state().position + type->offset;
          element.state().float_box.upper =
              element.state().float_box.lower + type->size;
        } else {
          assert(false);
        }
      }

      system(element).set_dependent_state(element);

      for (auto child = element.child(); child.exists(); child = child.next()) {
        stack.push(child);
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
        if (element_focus.exists()) {
          element_focus.set_dirty(
              system(element_focus).focus_leave(element_focus, false));
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

    if (!handled && element_focus.exists()) {
      element_focus.set_dirty(
          system(element_focus).key_event(element_focus, event));
    }
  }

  for (const auto& event : window.text_events()) {
    if (element_focus.exists()) {
      element_focus.set_dirty(
          system(element_focus).text_event(element_focus, event));
    }
  }
}

ElementPtr Gui::get_leaf_node(const Vecf& position) {
  auto get_tree_leaf = [this, &position](ElementPtr root) -> ElementPtr {
    ElementPtr leaf = ElementPtr();

    std::stack<ElementPtr> stack;
    stack.push(root);

    while (!stack.empty()) {
      auto element = stack.top();
      stack.pop();

      if (element.state().floating) {
        if (element != root) {
          continue;
        } else if (!element.state().float_box.contains(position)) {
          continue;
        }
      } else if (!element.state().box().contains(position)) {
        continue;
      }
      leaf = element;

      for (auto child = element.child(); child.exists(); child = child.next()) {
        stack.push(child);
      }
    }
    return leaf;
  };

  for (auto iter = floating_elements.rbegin(); iter != floating_elements.rend();
       iter++) {
    auto leaf = get_tree_leaf(*iter);
    if (leaf.exists()) {
      return leaf;
    }
  }
  return get_tree_leaf(tree.root());
}

void Gui::event_handling_left_click(const MouseEvent& event) {
  if (event.action != MouseAction::Press) {
    // Pass-through the hold or release event
    // node_focus should be a valid node, but there may be edge cases where
    // this isn't true (eg: The node gets removed)
    if (element_focus.exists()) {
      element_focus.set_dirty(
          system(element_focus).mouse_event(element_focus, event));
    }
    return;
  }

  // Clicked -> new focused node

  ElementPtr prev_element_focus = element_focus;
  element_focus = get_leaf_node(event.position);

  if (element_focus != prev_element_focus) {
    if (prev_element_focus.exists()) {
      prev_element_focus.set_dirty(
          system(prev_element_focus).focus_leave(prev_element_focus, true));
      set_tree_focus(prev_element_focus, false);
    }
    if (element_focus.exists()) {
      // Only use focus_enter() for non-click focus enter (ie: tab into the
      // element)
      set_tree_focus(element_focus, true);
    }
  }
  if (element_focus.exists()) {
    element_focus.set_dirty(
        system(element_focus).mouse_event(element_focus, event));
  }
}

void Gui::event_handling_hover(const Vecf& mouse_pos) {
  if (element_hover.exists()) {
    element_hover.state().hovered = false;
  }

  element_hover = get_leaf_node(mouse_pos);
  if (!element_hover.exists()) {
    return;
  }
  element_hover.state().hovered = true;
  system(element_hover).mouse_hover(element_hover, mouse_pos);
}

void Gui::event_handling_scroll(const ScrollEvent& event) {
  ElementPtr element = get_leaf_node(event.position);
  while (element.exists()) {
    if (system(element).scroll_event(element, event)) {
      return;
    }
    element = element.parent();
  }
}

void Gui::set_tree_focus(ElementPtr element, bool focused) {
  element.state().focused = focused;
  element.state().in_focus_tree = focused;

  bool found_floating = false;
  if (focused && element.state().floating) {
    found_floating = true;
    element.state().float_priority = next_float_priority++;
  }

  element = element.parent();
  while (element.exists()) {
    element.state().in_focus_tree = focused;
    if (focused && !found_floating && element.state().floating) {
      found_floating = true;
      element.state().float_priority = next_float_priority++;
    }
    element = element.parent();
  }
}

void Gui::focus_next(bool reverse) {
  if (!tree.root().exists()) {
    return;
  }
  auto next = element_focus;

  do {
    if (!reverse) {
      if (!next.exists()) {
        next = tree.root();
      } else if (next.child()) {
        next = next.child();
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
  } while (next && next != tree.root() && next.state().hidden);

  if (next == tree.root() && tree.root().state().hidden) {
    next = ElementPtr();
  }

  if (element_focus) {
    auto prev_element_focus = element_focus;
    prev_element_focus.set_dirty(
        system(prev_element_focus).focus_leave(prev_element_focus, true));
    set_tree_focus(element_focus, false);
  }

  element_focus = next;

  if (element_focus) {
    set_tree_focus(element_focus, true);
    system(element_focus).focus_enter(element_focus);
  }
}

} // namespace datagui

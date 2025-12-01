#include "datagui/gui.hpp"
#include <charconv>
#include <sstream>
#include <stack>

#include "datagui/system/button.hpp"
#include "datagui/system/checkbox.hpp"
#include "datagui/system/color_picker.hpp"
#include "datagui/system/dropdown.hpp"
#include "datagui/system/floating.hpp"
#include "datagui/system/labelled.hpp"
#include "datagui/system/section.hpp"
#include "datagui/system/series.hpp"
#include "datagui/system/slider.hpp"
#include "datagui/system/text_box.hpp"
#include "datagui/system/text_input.hpp"
#include "datagui/system/viewport.hpp"

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
  systems[(std::size_t)Type::ColorPicker] =
      std::make_unique<ColorPickerSystem>(fm, theme);
  systems[(std::size_t)Type::Dropdown] =
      std::make_unique<DropdownSystem>(fm, theme);
  systems[(std::size_t)Type::Floating] =
      std::make_unique<FloatingSystem>(fm, theme);
  systems[(std::size_t)Type::Labelled] =
      std::make_unique<LabelledSystem>(fm, theme);
  systems[(std::size_t)Type::Section] =
      std::make_unique<SectionSystem>(fm, theme);
  systems[(std::size_t)Type::Series] = std::make_unique<SeriesSystem>(theme);
  systems[(std::size_t)Type::Slider] =
      std::make_unique<SliderSystem>(fm, theme);
  systems[(std::size_t)Type::TextBox] =
      std::make_unique<TextBoxSystem>(fm, theme);
  systems[(std::size_t)Type::TextInput] =
      std::make_unique<TextInputSystem>(fm, theme);
  systems[(std::size_t)Type::ViewportPtr] = std::make_unique<ViewportSystem>();
}

bool Gui::running() const {
  return window.running();
}

void Gui::check_begin() {
  if (stack.empty()) {
    tree.poll();
    current = tree.root();
    var_current = VarPtr();
  }
}

void Gui::move_down() {
  current.clear_dependencies();
  stack.emplace(current, var_current);
  current = current.child();
  var_current = VarPtr();
}

void Gui::end() {
  current.expect_end();
  assert(!stack.empty());
  std::tie(current, var_current) = stack.top();
  stack.pop();
  if (current.type() == Type::ViewportPtr) {
    current.viewport().viewport->end();
  }
  current = current.next();
}

void Gui::poll() {
  assert(stack.empty());
  tree.clear_dirty();
  calculate_sizes();
  render();
  event_handling();
}

void Gui::button(
    const std::string& text,
    const std::function<void()>& callback) {
  current.expect(Type::Button, read_key());
  auto& button = current.button();
  args_.apply(current);
  current = current.next();

  button.text = text;
  button.callback = callback;
}

void Gui::checkbox(
    bool initial_value,
    const std::function<void(bool)>& callback) {
  bool is_new = current.expect(Type::Checkbox, read_key());
  auto& checkbox = current.checkbox();
  args_.apply(current);
  current = current.next();

  checkbox.callback = callback;
  if (is_new || overwrite) {
    checkbox.checked = initial_value;
  }
}

void Gui::checkbox(const Var<bool>& var) {
  current.expect(Type::Checkbox, read_key());
  auto& checkbox = current.checkbox();
  args_.apply(current);
  current = current.next();

  checkbox.callback = [var](bool value) { var.set(value); };
  checkbox.checked = *var;
}

void Gui::color_picker(
    const Color& initial_value,
    const std::function<void(const Color&)>& callback) {
  bool is_new = current.expect(Type::ColorPicker, read_key());
  auto& color_picker = current.color_picker();
  args_.apply(current);
  current = current.next();

  color_picker.callback = callback;
  if (is_new || overwrite) {
    color_picker.value = initial_value;
  }
}

void Gui::color_picker(const Var<Color>& var) {
  current.expect(Type::ColorPicker, read_key());
  auto& color_picker = current.color_picker();
  args_.apply(current);
  current = current.next();

  color_picker.callback = [var](const Color& value) { var.set(value); };
  color_picker.value = *var;
}

void Gui::dropdown(
    const std::vector<std::string>& choices,
    int initial_choice,
    const std::function<void(int)>& callback) {
  bool is_new = current.expect(Type::Dropdown, read_key());
  auto& dropdown = current.dropdown();
  args_.apply(current);
  current = current.next();

  if (is_new || overwrite) {
    dropdown.choice = initial_choice;
  }
  dropdown.choices = choices;
  dropdown.callback = callback;
}

void Gui::dropdown(
    const std::vector<std::string>& choices,
    const Var<int>& var) {
  current.expect(Type::Dropdown, read_key());
  auto& dropdown = current.dropdown();
  args_.apply(current);
  current = current.next();

  dropdown.choices = choices;
  dropdown.choice = *var;
  dropdown.callback = [var](int value) { var.set(value); };
}

bool Gui::floating(
    const Var<bool>& open_var,
    const std::string& title,
    float width,
    float height) {
  check_begin();
  current.expect(Type::Floating, read_key());
  args_.apply(current);
  auto& floating = current.floating();

  floating.title = title;
  floating.width = width;
  floating.height = height;
  floating.closed_callback = [open_var]() { open_var.set(false); };

  depend_variable(open_var);
  if (*open_var != floating.open) {
    floating.open = *open_var;
  }

  if (!floating.open && !current.state().hidden) {
    current.clear_children();
  }
  current.state().hidden = !floating.open;

  if (!floating.open || !current.dirty()) {
    current = current.next();
    return false;
  }

  move_down();
  return true;
}

bool Gui::labelled(const std::string& label) {
  check_begin();
  current.expect(Type::Labelled, read_key());
  args_.apply(current);
  auto& labelled = current.labelled();
  labelled.label = label;

  if (current.dirty()) {
    move_down();
    return true;
  }
  current = current.next();
  return false;
}

bool Gui::section(const std::string& label) {
  check_begin();
  current.expect(Type::Section, read_key());
  args_.apply(current);
  auto& section = current.section();
  section.label = label;

  if (current.dirty()) {
    move_down();
    return true;
  }
  current = current.next();
  return false;
}

bool Gui::series() {
  check_begin();
  current.expect(Type::Series, read_key());
  args_.apply(current);

  if (current.dirty()) {
    move_down();
    return true;
  }
  current = current.next();
  return false;
}

template <typename T>
void Gui::slider(
    T initial_value,
    T lower,
    T upper,
    const std::function<void(T)>& callback) {
  bool is_new = current.expect(Type::Slider, read_key());
  auto& slider = current.slider();
  args_.apply(current);
  current = current.next();

  slider.type = number_type<T>();
  slider.lower = lower;
  slider.upper = upper;

  if (is_new || overwrite) {
    slider.value = initial_value;
  }
  if constexpr (std::is_same_v<T, double>) {
    slider.callback = callback;
  }
  if constexpr (!std::is_same_v<T, double>) {
    slider.callback = [callback](double value) { callback(value); };
  }
}

template <typename T>
void Gui::slider(T lower, T upper, const Var<T>& var) {
  current.expect(Type::Slider, read_key());
  auto& slider = current.slider();
  args_.apply(current);
  current = current.next();

  slider.type = number_type<T>();
  slider.lower = lower;
  slider.upper = upper;

  slider.callback = [var](double value) { var.set(value); };
  slider.value = *var;
}

#define INSTANTIATE(T) \
  template void Gui::slider<T>(T, T, T, const std::function<void(T)>&); \
  template void Gui::slider<T>(T, T, const Var<T>&);
INSTANTIATE(std::int32_t)
INSTANTIATE(std::int64_t)
INSTANTIATE(std::uint32_t)
INSTANTIATE(std::uint64_t)
INSTANTIATE(float)
INSTANTIATE(double)
INSTANTIATE(std::uint8_t)
#undef INSTANTIATE

void Gui::text_input(
    const std::string& initial_value,
    const std::function<void(const std::string&)>& callback) {
  bool is_new = current.expect(Type::TextInput, read_key());
  auto& text_input = current.text_input();
  args_.apply(current);
  current = current.next();

  if (is_new || overwrite) {
    text_input.text = initial_value;
  }
  text_input.callback = callback;
}

void Gui::text_input(const Var<std::string>& var) {
  current.expect(Type::TextInput, read_key());
  auto& text_input = current.text_input();
  args_.apply(current);
  current = current.next();

  text_input.callback = [var](const std::string& value) { var.set(value); };
  text_input.text = *var;
}

template <typename T>
void Gui::number_input(
    const T& initial_value,
    const std::function<void(T callback)>& callback) {
  bool is_new = current.expect(Type::TextInput, read_key());
  auto& text_input = current.text_input();
  args_.apply(current);
  text_input.number_type = number_type<T>();
  current = current.next();

  if (is_new || overwrite) {
    text_input.text = std::to_string(initial_value);
  }
  text_input.callback = [callback](const std::string& value) {
    T number;
    if (text_to_number(value, number)) {
      callback(number);
    }
  };
}

template <typename T>
void Gui::number_input(const Var<T>& var) {
  current.expect(Type::TextInput, read_key());
  auto& text_input = current.text_input();
  args_.apply(current);
  text_input.number_type = number_type<T>();
  current = current.next();

  text_input.callback = [var](const std::string& value) {
    T number;
    if (text_to_number(value, number)) {
      var.set(number);
    }
  };
  text_input.text = std::to_string(*var);
}

#define INSTANTIATE(T) \
  template void Gui::number_input<T>(const T&, const std::function<void(T)>&); \
  template void Gui::number_input<T>(const Var<T>&);
INSTANTIATE(std::int32_t)
INSTANTIATE(std::int64_t)
INSTANTIATE(std::uint32_t)
INSTANTIATE(std::uint64_t)
INSTANTIATE(float)
INSTANTIATE(double)
INSTANTIATE(std::uint8_t)
#undef INSTANTIATE

void Gui::text_box(const std::string& text) {
  current.expect(Type::TextBox, read_key());
  auto& text_box = current.text_box();
  args_.apply(current);
  current = current.next();

  text_box.text = text;
}

void Gui::render() {
  auto render_tree = [this](ConstElementPtr root) {
    struct State {
      ConstElementPtr element;
      bool first_visit;
      State(ConstElementPtr element) : element(element), first_visit(true) {}
    };
    std::stack<State> stack;
    stack.emplace(root);

    while (!stack.empty()) {
      auto& state = stack.top();
      const auto& element = state.element;
      assert(element);

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

      render(element);
      renderer.push_mask(element.state().child_mask);

      for (auto child = element.child(); child; child = child.next()) {
        stack.emplace(child);
      }
    }
  };

  window.render_begin();
  renderer.render_begin(window.size());

  render_tree(tree.root());

  for (auto element : floating_elements) {
    renderer.new_layer();
    render_tree(element);
  }
#ifdef DATAGUI_DEBUG
  if (debug_mode_) {
    renderer.new_layer();
    debug_render();
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

    Color debug_color = element.state().focused         ? Color::Blue()
                        : element.state().in_focus_tree ? Color::Red()
                                                        : Color::Green();
    renderer.queue_box(
        Box2(
            element.state().position,
            element.state().position + element.state().size),
        Color::Clear(),
        2,
        debug_color);

    if (element.state().floating) {
      renderer.queue_box(
          element.state().float_box,
          Color::Clear(),
          2,
          element.state().in_focus_tree ? Color(1, 0, 1) : Color(0, 1, 1));
    }

    renderer.push_mask(element.state().child_mask);

    for (auto child = element.child(); child; child = child.next()) {
      layer_stack.push(child);
    }

    if (element.state().focused) {
      focused = element;
    }
  }

  if (focused) {
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
        Box2(
            window.size() - text_size - Vec2::uniform(15),
            window.size() - Vec2::uniform(5)),
        Color::White(),
        2,
        Color::Black());
    renderer.queue_text(
        window.size() - text_size - Vec2::uniform(10),
        debug_text,
        Font::DejaVuSans,
        24,
        Color::Black());
  }
}
#endif

void Gui::calculate_sizes() {
  if (!tree.root()) {
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
      if (element.child() && state.first_visit) {
        state.first_visit = false;
        for (auto child = element.child(); child; child = child.next()) {
          stack.emplace(child);
        }
        continue;
      }
      stack.pop();

      set_input_state(element);
    }
  }

  {
    std::stack<ElementPtr> stack;
    {
      auto root = tree.root();
      assert(root);
      root.state().position = Vec2();
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

      set_dependent_state(element);

      for (auto child = element.child(); child; child = child.next()) {
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
        if (element_focus) {
          if (focus_leave(element_focus, false)) {
            element_focus.set_dirty(true);
          }
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
      if (key_event(element_focus, event)) {
        element_focus.set_dirty(true);
      }
    }
  }

  if (element_focus) {
    for (const auto& event : window.text_events()) {
      if (text_event(element_focus, event)) {
        element_focus.set_dirty(true);
      }
    }
  }
}

ElementPtr Gui::get_leaf_node(const Vec2& position) {
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

      for (auto child = element.child(); child; child = child.next()) {
        stack.push(child);
      }
    }
    return leaf;
  };

  for (auto iter = floating_elements.rbegin(); iter != floating_elements.rend();
       iter++) {
    auto leaf = get_tree_leaf(*iter);
    if (leaf) {
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
    if (element_focus && mouse_event(element_focus, event)) {
      element_focus.set_dirty(true);
    }
    return;
  }

  // Clicked -> new focused node

  ElementPtr prev_element_focus = element_focus;
  element_focus = get_leaf_node(event.position);

  if (element_focus != prev_element_focus) {
    if (prev_element_focus) {
      if (focus_leave(prev_element_focus, true)) {
        prev_element_focus.set_dirty(true);
      }
      set_tree_focus(prev_element_focus, false);
    }
    if (element_focus) {
      // Only use focus_enter() for non-click focus enter (ie: tab into the
      // element)
      set_tree_focus(element_focus, true);
    }
  }
  if (element_focus && mouse_event(element_focus, event)) {
    element_focus.set_dirty(true);
  }
}

void Gui::event_handling_hover(const Vec2& mouse_pos) {
  if (element_hover) {
    element_hover.state().hovered = false;
  }

  element_hover = get_leaf_node(mouse_pos);
  if (!element_hover) {
    return;
  }
  element_hover.state().hovered = true;
  mouse_hover(element_hover, mouse_pos);
}

void Gui::event_handling_scroll(const ScrollEvent& event) {
  ElementPtr element = get_leaf_node(event.position);
  while (element) {
    if (scroll_event(element, event)) {
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
  while (element) {
    element.state().in_focus_tree = focused;
    if (focused && !found_floating && element.state().floating) {
      found_floating = true;
      element.state().float_priority = next_float_priority++;
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
    if (focus_leave(prev_element_focus, true)) {
      prev_element_focus.set_dirty(true);
    }
    set_tree_focus(element_focus, false);
  }

  element_focus = next;

  if (element_focus) {
    set_tree_focus(element_focus, true);
    focus_enter(element_focus);
  }
}

} // namespace datagui

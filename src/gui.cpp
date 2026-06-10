#include "datagui/gui.hpp"
#include <sstream>
#include <stack>

#include "datagui/system/button.hpp"
#include "datagui/system/checkbox.hpp"
#include "datagui/system/collapsable.hpp"
#include "datagui/system/color_picker.hpp"
#include "datagui/system/dropdown.hpp"
#include "datagui/system/group.hpp"
#include "datagui/system/popup.hpp"
#include "datagui/system/select.hpp"
#include "datagui/system/slider.hpp"
#include "datagui/system/split.hpp"
#include "datagui/system/tabs.hpp"
#include "datagui/system/text_box.hpp"
#include "datagui/system/text_input.hpp"
#include "datagui/system/viewport_ptr.hpp"

namespace datagui {

using namespace std::placeholders;

Gui::Gui(const std::string& title, std::size_t width, std::size_t height) :
    window(title, width, height) {
  fm = std::make_shared<FontManager>();
  theme = std::make_shared<Theme>(theme_default());
  renderer.init(fm);

  systems.resize(TypeCount);

#define REGISTER(Name, System, args...) \
  systems[(std::size_t)Type::Name] = std::make_unique<System>(args);

  REGISTER(Button, ButtonSystem, fm, theme);
  REGISTER(Checkbox, CheckboxSystem, fm, theme);
  REGISTER(Collapsable, CollapsableSystem, fm, theme);
  REGISTER(ColorPicker, ColorPickerSystem, fm, theme);
  REGISTER(Dropdown, DropdownSystem, fm, theme);
  REGISTER(Group, GroupSystem, theme);
  REGISTER(Popup, PopupSystem, fm, theme);
  REGISTER(Select, SelectSystem, fm, theme);
  REGISTER(Slider, SliderSystem, fm, theme);
  REGISTER(Split, SplitSystem, theme);
  REGISTER(Tabs, TabsSystem, fm, theme);
  REGISTER(TextBox, TextBoxSystem, fm, theme);
  REGISTER(TextInput, TextInputSystem, fm, theme);
  REGISTER(ViewportPtr, ViewportPtrSystem);

#undef REGISTER
  for (const auto& system : systems) {
    assert(system);
  }
}

void Gui::move_down() {
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

bool Gui::poll() {
  assert(stack.empty());
  calculate_sizes();
  render();
  event_handling();

  if (!window.running()) {
    return false;
  }

  current = tree.root();
  var_current = VarPtr();

  return true;
}

bool Gui::button(const std::string& text) {
  bool is_new = current.expect(Type::Button, read_key());
  auto& button = current.button();
  if (is_new) {
    button.text = text;
  }
  args_.apply(current);
  current = current.next();

  if (button.released) {
    button.released = false;
    return true;
  }
  return false;
}

std::optional<bool> Gui::checkbox(bool initial_value) {
  bool is_new = current.expect(Type::Checkbox, read_key());
  auto& checkbox = current.checkbox();
  if (is_new) {
    checkbox.checked = initial_value;
  }
  args_.apply(current);
  current = current.next();

  if (checkbox.changed) {
    checkbox.changed = false;
    return checkbox.checked;
  }
  return std::nullopt;
}

bool Gui::checkbox_v(bool& value) {
  current.expect(Type::Checkbox, read_key());
  auto& checkbox = current.checkbox();
  args_.apply(current);
  current = current.next();

  if (checkbox.changed) {
    checkbox.changed = false;
    value = checkbox.checked;
    return true;
  } else {
    checkbox.checked = value;
    return false;
  }
}

bool Gui::collapsable(const std::string& label) {
  current.expect(Type::Collapsable, read_key());
  args_.apply(current);
  auto& collapsable = current.collapsable();
  collapsable.label = label;

  if (collapsable.open) {
    move_down();
    return true;
  }
  current = current.next();
  return false;
}

std::optional<Color> Gui::color_picker(const Color& initial_value) {
  bool is_new = current.expect(Type::ColorPicker, read_key());
  auto& color_picker = current.color_picker();
  if (is_new) {
    color_picker.value = initial_value;
  }
  args_.apply(current);
  current = current.next();

  if (color_picker.changed) {
    color_picker.changed = false;
    return color_picker.value;
  }
  return std::nullopt;
}

bool Gui::color_picker_v(Color& value) {
  current.expect(Type::ColorPicker, read_key());
  auto& color_picker = current.color_picker();
  args_.apply(current);
  current = current.next();

  if (color_picker.modified) {
    if (color_picker.always) {
      value = color_picker.value;
      return true;
    } else {
      return false;
    }
  }
  if (color_picker.changed) {
    color_picker.changed = false;
    value = color_picker.value;
    return true;
  } else {
    color_picker.value = value;
    return false;
  }
}

bool Gui::dropdown(const std::string& label) {
  current.expect(Type::Dropdown, read_key());
  args_.apply(current);
  auto& dropdown = current.dropdown();

  dropdown.label = label;

  if (!dropdown.open) {
    if (!dropdown.retain) {
      current.clear();
    }
    current = current.next();
    return false;
  }

  move_down();
  return true;
}

void Gui::group() {
  current.expect(Type::Group, read_key());
  args_.apply(current);
  move_down();
}

bool Gui::popup(
    bool& open,
    const std::string& title,
    float width,
    float height) {
  current.expect(Type::Popup, read_key());
  args_.apply(current);
  auto& popup = current.popup();

  popup.title = title;
  popup.popup_size = Vec2(width, height);

  if (popup.close_button_released) {
    popup.close_button_released = false;
    open = false;
    popup.open = false;
  } else {
    popup.open = open;
  }
  if (popup.open) {
    move_down();
    return true;
  }
  if (current.child() && !popup.retain) {
    current.clear();
  }
  current = current.next();
  return false;
}

std::optional<int> Gui::select(
    int initial_choice,
    const std::vector<std::string>& choices) {
  bool is_new = current.expect(Type::Select, read_key());
  auto& select = current.select();
  if (is_new) {
    select.choice = initial_choice;
  }
  select.choices = choices;
  if (select.choice >= choices.size()) {
    select.choice = std::max(1ul, choices.size()) - 1;
  }
  args_.apply(current);
  current = current.next();

  if (select.changed) {
    select.changed = false;
    return select.choice;
  }
  return std::nullopt;
}

bool Gui::select_v(int& choice, const std::vector<std::string>& choices) {
  current.expect(Type::Select, read_key());
  auto& select = current.select();
  args_.apply(current);
  current = current.next();

  if (select.changed) {
    select.changed = true;
    choice = select.choice;
    return true;
  } else {
    select.choice = choice;
    return false;
  }
}

template <typename T>
std::optional<T> Gui::slider(T initial_value, T lower, T upper) {
  bool is_new = current.expect(Type::Slider, read_key());
  auto& slider = current.slider();
  slider.type = number_type<T>();
  slider.lower = static_cast<T>(lower);
  slider.upper = static_cast<T>(upper);
  if (is_new) {
    slider.value = std::clamp(
        static_cast<double>(initial_value),
        slider.lower,
        slider.upper);
  }
  args_.apply(current);

  current = current.next();

  if (slider.value < slider.lower || slider.value > slider.upper) {
    slider.value = std::clamp(slider.value, slider.lower, slider.upper);
    slider.changed = true;
  }

  if (slider.changed) {
    slider.changed = false;
    return static_cast<T>(slider.value);
  }
  return std::nullopt;
}

template <typename T>
bool Gui::slider_v(T& value, T lower, T upper) {
  current.expect(Type::Slider, read_key());
  auto& slider = current.slider();
  slider.type = number_type<T>();
  slider.lower = static_cast<T>(lower);
  slider.upper = static_cast<T>(upper);
  args_.apply(current);

  current = current.next();

  if (!slider.changed) {
    slider.value = static_cast<double>(value);
    if (slider.value >= slider.lower && slider.value <= slider.upper) {
      return false;
    }
    slider.value = std::clamp(slider.value, slider.lower, slider.upper);
  }
  // Changed or clamped

  value = static_cast<double>(slider.value);
  return true;
}

#define INSTANTIATE(T) \
  template std::optional<T> Gui::slider<T>(T, T, T); \
  template bool Gui::slider_v<T>(T&, T, T);
INSTANTIATE(std::int32_t)
INSTANTIATE(std::int64_t)
INSTANTIATE(std::uint32_t)
INSTANTIATE(std::uint64_t)
INSTANTIATE(float)
INSTANTIATE(double)
INSTANTIATE(std::uint8_t)
#undef INSTANTIATE

void Gui::hsplit(float ratio) {
  bool is_new = current.expect(Type::Split, read_key());
  auto& split = current.split();
  args_.apply(current);

  if (is_new) {
    split.ratio = ratio;
  }
  split.direction = Direction::Horizontal;

  move_down();
}

void Gui::vsplit(float ratio) {
  bool is_new = current.expect(Type::Split, read_key());
  auto& split = current.split();
  args_.apply(current);

  if (is_new) {
    split.ratio = ratio;
  }
  split.direction = Direction::Vertical;

  move_down();
}

void Gui::tabs(size_t initial_tab) {
  bool is_new = current.expect(Type::Tabs, read_key());
  auto& tabs = current.tabs();
  args_.apply(current);
  if (is_new) {
    tabs.tab = initial_tab;
  }
  if (!is_new && tabs.tab >= tabs.labels.size()) {
    tabs.tab = std::max(1ul, tabs.labels.size()) - 1;
  }
  tabs.labels.clear();
  move_down();
}

bool Gui::tab_group(const std::string& label) {
  current.expect(Type::Group, read_key());
  args_.apply(current);

  auto parent = current.parent();
  assert(parent && parent.type() == Type::Tabs);
  auto& tabs = parent.tabs();

  size_t index = tabs.labels.size();
  tabs.labels.push_back(label);
  if (tabs.tab == index) {
    current.state().hidden = false;
    move_down();
    return true;
  }
  current.state().hidden = true;
  current = current.next();
  return false;
}

const std::string* Gui::text_input(const std::string& initial_value) {
  bool is_new = current.expect(Type::TextInput, read_key());
  auto& text_input = current.text_input();
  if (is_new) {
    text_input.text = initial_value;
  }
  args_.apply(current);
  current = current.next();

  if (text_input.changed) {
    text_input.changed = false;
    return &text_input.text;
  }
  return nullptr;
}

bool Gui::text_input_v(std::string& value) {
  current.expect(Type::TextInput, read_key());
  auto& text_input = current.text_input();
  args_.apply(current);
  current = current.next();

  if (text_input.changed) {
    text_input.changed = false;
    value = text_input.text;
    return true;
  } else {
    text_input.text = value;
    return false;
  }
}

template <typename T>
std::optional<T> Gui::number_input(T initial_value) {
  bool is_new = current.expect(Type::TextInput, read_key());
  auto& text_input = current.text_input();
  if (is_new) {
    text_input.text = std::to_string(initial_value);
  }
  args_.apply(current);
  text_input.number_type = number_type<T>();
  current = current.next();

  if (text_input.changed) {
    text_input.changed = false;
    T number;
    if (text_to_number(text_input.text, number)) {
      return number;
    }
  }
  return std::nullopt;
}

template <typename T>
bool Gui::number_input_v(T& value) {
  current.expect(Type::TextInput, read_key());
  auto& text_input = current.text_input();
  args_.apply(current);
  text_input.number_type = number_type<T>();
  current = current.next();

  if (text_input.changed) {
    text_input.changed = false;
    T number;
    if (text_to_number(text_input.text, number)) {
      value = number;
      return true;
    }
  }
  text_input.text = std::to_string(value);
  return false;
}

#define INSTANTIATE(T) \
  template std::optional<T> Gui::number_input<T>(T); \
  template bool Gui::number_input_v<T>(T&);
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
    if (!root) {
      return;
    }
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
  renderer.begin(Box2(Vec2(), window.size()));

  render_tree(tree.root());
  renderer.render();

  for (auto element : ordered_floating_elements) {
    render_tree(element);
    renderer.render();
  }
#ifdef DATAGUI_DEBUG
  if (debug_mode_) {
    debug_render();
    renderer.render();
  }
#endif

  renderer.end();
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

  auto prev_floating_elements = floating_elements;
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

      // Special case: Fixed size if root is a viewport
      if (root.type() == Type::ViewportPtr) {
        window.set_fixed_size(root.state().fixed_size);
      } else {
        window.set_dynamic_size();
      }
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
        if (!prev_floating_elements.contains(element)) {
          element.state().float_priority = next_float_priority++;
        }
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

  ordered_floating_elements.clear();
  for (auto element : floating_elements) {
    ordered_floating_elements.insert(element);
  }
}

void Gui::event_handling() {
  window.poll_events();

  for (const auto& event : window.mouse_events()) {
    switch (event.button) {
    case MouseButton::Left:
      event_handling_left_click(event);
      break;
    case MouseButton::Right:
      event_handling_right_click(event);
      break;
    case MouseButton::Middle:
      event_handling_middle_click(event);
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
        focus_next(event.mod.shift);
        handled = true;
        break;
      case Key::Escape:
        change_tree_focus(element_focus, ElementPtr());
        handled = true;
        break;
#ifdef DATAGUI_DEBUG
      case Key::D:
        if (event.mod.ctrl) {
          handled = true;
          debug_mode_ = !debug_mode_;
        }
#endif
      default:
        break;
      }
    }

    if (!handled && element_focus) {
      key_event(element_focus, event);
    }
  }

  if (element_focus) {
    for (const auto& event : window.text_events()) {
      text_event(element_focus, event);
    }
  }

  for (auto callback : misc_events) {
    callback();
  }
  misc_events.clear();
}

ElementPtr Gui::get_leaf_node(const Vec2& position) {
  auto get_tree_leaf = [this, &position](ElementPtr root) -> ElementPtr {
    if (!root) {
      return ElementPtr();
    }
    ElementPtr leaf = ElementPtr();

    std::stack<ElementPtr> stack;
    stack.push(root);

    while (!stack.empty()) {
      auto element = stack.top();
      stack.pop();

      if (element.state().hidden) {
        continue;
      }
      bool contains = false;
      if (!element.state().float_only) {
        contains |= element.state().box().contains(position);
      }
      if (element.state().floating) {
        contains |= element.state().float_box.contains(position);
      }
      if (!contains) {
        continue;
      }
      leaf = element;

      for (auto child = element.child(); child; child = child.next()) {
        stack.push(child);
      }
    }
    return leaf;
  };

  for (auto iter = ordered_floating_elements.rbegin();
       iter != ordered_floating_elements.rend();
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
    if (element_focus) {
      mouse_event(element_focus, event);
    }
    return;
  }

  // Clicked -> new focused node

  ElementPtr prev_element_focus = element_focus;
  element_focus = get_leaf_node(event.position);

  change_tree_focus(prev_element_focus, element_focus);
  if (element_focus) {
    mouse_event(element_focus, event);
  }
}

void Gui::event_handling_right_click(const MouseEvent& event) {
  if (event.action == MouseAction::Press) {
    element_left_held = get_leaf_node(event.position);
    if (element_left_held) {
      mouse_event(element_left_held, event);
    }
    return;
  }
  if (!element_left_held) {
    return;
  }
  mouse_event(element_left_held, event);

  if (event.action == MouseAction::Release) {
    element_left_held = ElementPtr();
  }
}

void Gui::event_handling_middle_click(const MouseEvent& event) {
  if (event.action == MouseAction::Press) {
    element_middle_held = get_leaf_node(event.position);
    if (element_middle_held) {
      mouse_event(element_middle_held, event);
    }
    return;
  }
  if (!element_middle_held) {
    return;
  }
  mouse_event(element_middle_held, event);

  if (event.action == MouseAction::Release) {
    element_middle_held = ElementPtr();
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

void Gui::change_tree_focus(ElementPtr from, ElementPtr to) {
  std::unordered_set<ElementPtr, ElementPtr::HashFunc> removed;
  std::unordered_set<ElementPtr, ElementPtr::HashFunc> added;

  if (from) {
    from.state().focused = false;
    auto iter = from;
    while (iter) {
      removed.insert(iter);
      iter.state().in_focus_tree = false;
      iter = iter.parent();
    }
  }

  if (to) {
    bool found_floating = false;
    to.state().focused = true;
    auto iter = to;
    while (iter) {
      added.insert(iter);
      iter.state().in_focus_tree = true;
      if (!found_floating && iter.state().floating) {
        found_floating = true;
        iter.state().float_priority = next_float_priority++;
      }
      iter = iter.parent();
    }
  }

  if (from) {
    focus_leave(from, true);
  }
  for (auto iter : removed) {
    if (!added.contains(iter)) {
      focus_tree_leave(iter);
    }
  }
  if (to) {
    focus_enter(to);
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

  change_tree_focus(element_focus, next);
}

template <typename T>
requires std::is_base_of_v<Viewport, T>
T& Gui::viewport(float width, float height) {
  current.expect(Type::ViewportPtr, read_key());
  auto& viewport = current.viewport();
  if (!viewport.viewport) {
    viewport.viewport = std::make_unique<T>();
    viewport.viewport->init(width, height, theme, fm);
  }
  // Renderered width/height can differ to the initial width/height
  // above - this defines the size used for the framebuffer
  viewport.width = width;
  viewport.height = height;

  move_down();
  viewport.viewport->begin();
  T* ptr = dynamic_cast<T*>(viewport.viewport.get());
  assert(ptr);
  return *ptr;
}
template Canvas2d& Gui::viewport<Canvas2d>(float, float);
template Canvas3d& Gui::viewport<Canvas3d>(float, float);
template Plotter& Gui::viewport<Plotter>(float, float);

} // namespace datagui

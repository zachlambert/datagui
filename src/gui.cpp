#include "datagui/gui.hpp"
#include <sstream>
#include <stack>
#include <unordered_set>

namespace dgui {

using namespace std::placeholders;

Gui::Gui() {}

Gui::~Gui() {
  close();
}

void Gui::open(
    const std::string& title,
    std::size_t width,
    std::size_t height) {
  window.open(title, width, height);

  font_registry = std::make_shared<FontRegistry>();
  theme = std::make_shared<Theme>(theme_default());
  program_registry.init();
  font_registry->init();
  systems.init(font_registry, theme);
}

void Gui::close() {
  window.close();
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
  if (element_focus && tree.has_removed(element_focus)) {
    element_focus = ElementPtr();
  }
  tree.clear_removed();

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
  bool is_new = current.expect(Type::Collapsable, read_key());
  args_.apply(current);
  auto& collapsable = current.collapsable();
  collapsable.label = label;

  if (collapsable.open || is_new) {
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
  if (select.choice >= 0 &&
      static_cast<size_t>(select.choice) >= choices.size()) {
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
    select.changed = false;
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
    move_down();
    return true;
  }
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
  if (!tree.root()) {
    return;
  }

  auto render_layer = [this](ConstElementPtr layer_root, bool from_content) {
    assert(layer_root.state().display_mode != DisplayMode::Disabled);

    if (!from_content) {
      dl.new_group(layer_root.state().box());
      systems.render(layer_root, dl);
      if (layer_root.state().content_mode != DisplayMode::Inline) {
        return;
      }
    } else {
      dl.new_group(layer_root.state().content_box);
    }

    std::stack<std::pair<ConstElementPtr, Box2>> group_stack;
    group_stack.emplace(layer_root, layer_root.state().content_box);

    while (!group_stack.empty()) {
      auto [group_root, group_box] = group_stack.top();
      group_stack.pop();

      if (group_root != layer_root ||
          (!from_content && layer_root.state().content_overflowed)) {
        dl.new_group(group_box);
      }

      std::stack<ConstElementPtr> stack;
      stack.push(group_root);
      while (!stack.empty()) {
        auto element = stack.top();
        stack.pop();

        systems.render_content(element, dl);
        for (auto child = element.child(); child; child = child.next()) {
          const auto& c_state = child.state();
          if (c_state.display_mode != DisplayMode::Inline) {
            continue;
          }
          systems.render(child, dl);
          if (c_state.content_mode != DisplayMode::Inline) {
            continue;
          }
          if (c_state.content_overflowed) {
            group_stack.emplace(
                child,
                intersection(group_box, c_state.content_box));
          } else {
            stack.push(child);
          }
        }
      }
    }
  };

  dl.clear();
  render_layer(tree.root(), false);
  for (const auto& [layer, _] : layers_ordered) {
    render_layer(layer.element, layer.is_content);
  }
#ifdef DGUI_DEBUG
  if (debug_mode_) {
    debug_render();
  }
#endif

  window.render_begin();
  executor.draw(window.size(), program_registry, dl);
  window.render_end();
}

#ifdef DGUI_DEBUG
void Gui::debug_render() {
  dl.new_group(Box2(Vec2(), window.size()));

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

    if (element.state().display_mode == DisplayMode::Disabled) {
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
    dl.draw_box(
        Box2(
            element.state().position,
            element.state().position + element.state().size),
        Color::Clear(),
        2,
        debug_color);

    if (element.state().content_mode != DisplayMode::Disabled) {
      dl.draw_box(
          element.state().content_box,
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

    ss << "mouse pos: " << window.mouse_pos().x << ", " << window.mouse_pos().y;
    ss << "\nfixed: " << focused.state().fixed_size.x << ", "
       << focused.state().fixed_size.y;
    ss << "\ndynamic: " << focused.state().dynamic_size.x << ", "
       << focused.state().dynamic_size.y;
    ss << "\nsize: " << focused.state().size.x << ", "
       << focused.state().size.y;
    std::string debug_text = ss.str();

    const auto& font =
        font_registry->get_font(theme->text_font, theme->text_size);
    auto text_size = font.text_size(debug_text, LengthWrap());

    dl.draw_box(
        Box2(
            window.size() - text_size - Vec2::uniform(15),
            window.size() - Vec2::uniform(5)),
        Color::White(),
        2,
        Color::Black());
    dl.draw_text(
        font,
        window.size() - text_size - Vec2::uniform(10),
        Color::Black(),
        LengthWrap(),
        debug_text);
  }
}
#endif

void Gui::calculate_sizes() {
  if (!tree.root()) {
    return;
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

      if (element.child() && state.first_visit) {
        state.first_visit = false;
        for (auto child = element.child(); child; child = child.next()) {
          stack.emplace(child);
        }
        continue;
      }
      stack.pop();

      element.state().reset_input();
      systems.set_input_state(element);
    }
  }

  for (auto& [_, state] : layers) {
    state.visited = false;
  }
  auto visit_layer = [&](const ElementPtr& element, bool is_content) {
    Layer layer(element, is_content);
    auto iter = layers.find(layer);
    if (iter != layers.end()) {
      iter->second.visited = true;
      return;
    }
    layers.emplace(layer, next_z_order++);
  };
  {
    systems.set_window_box(Box2(Vec2(), window.size()));

    std::stack<ElementPtr> stack;
    {
      auto root = tree.root();
      assert(root);
      window.set_dynamic_size();
      root.state().position = Vec2();
      root.state().size = window.size();
      stack.push(root);
    }

    while (!stack.empty()) {
      auto element = stack.top();
      auto& state = element.state();
      stack.pop();

      if (state.display_mode == DisplayMode::Disabled) {
        continue;
      }
      if (state.display_mode == DisplayMode::Float) {
        visit_layer(element, false);
      }
      if (state.content_mode == DisplayMode::Float) {
        visit_layer(element, true);
      }

      systems.set_dependent_state(element);

      for (auto child = element.child(); child; child = child.next()) {
        stack.push(child);
      }
    }
  }
  layers_ordered.clear();
  for (const auto& layer_pair : layers) {
    layers_ordered.insert(layer_pair);
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
#ifdef DGUI_DEBUG
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
      systems.key_event(element_focus, event);
    }
  }

  if (element_focus) {
    for (const auto& event : window.text_events()) {
      systems.text_event(element_focus, event);
    }
  }

  for (auto callback : misc_events) {
    callback();
  }
  misc_events.clear();
}

ElementPtr Gui::get_leaf_node(const Vec2& position) {
  auto get_tree_leaf =
      [this, &position](ElementPtr root, bool from_content) -> ElementPtr {
    ElementPtr leaf = ElementPtr();
    if (!from_content && root.state().box().contains(position)) {
      leaf = root;
    }
    if (!from_content && root.state().content_mode != DisplayMode::Inline) {
      return leaf;
    }
    if (!root.state().content_box.contains(position)) {
      return leaf;
    }
    leaf = root;

    std::stack<ElementPtr> stack;
    stack.push(root);
    for (auto child = root.child(); child; child = child.next()) {
      stack.push(child);
    }

    while (!stack.empty()) {
      auto element = stack.top();
      const auto& state = element.state();
      stack.pop();

      if (state.display_mode != DisplayMode::Inline) {
        continue;
      }
      if (element.state().box().contains(position)) {
        leaf = element;
      }
      if (element.state().content_mode != DisplayMode::Inline ||
          !element.state().content_box.contains(position)) {
        continue;
      }
      leaf = element;

      for (auto child = element.child(); child; child = child.next()) {
        stack.push(child);
      }
    }
    return leaf;
  };

  for (auto iter = layers_ordered.rbegin(); iter != layers_ordered.rend();
       iter++) {
    const auto& layer = iter->first;
    auto leaf = get_tree_leaf(layer.element, layer.is_content);
    if (leaf) {
      return leaf;
    }
  }
  if (tree.root()) {
    return get_tree_leaf(tree.root(), false);
  }
  return ElementPtr();
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

  ElementPtr prev_element_focus = element_focus;
  element_focus = get_leaf_node(event.position);

  change_tree_focus(prev_element_focus, element_focus);
  if (element_focus) {
    systems.mouse_event(element_focus, event);
  }
}

void Gui::event_handling_right_click(const MouseEvent& event) {
  if (event.action == MouseAction::Press) {
    element_left_held = get_leaf_node(event.position);
    if (element_left_held) {
      systems.mouse_event(element_left_held, event);
    }
    return;
  }
  if (!element_left_held) {
    return;
  }
  systems.mouse_event(element_left_held, event);

  if (event.action == MouseAction::Release) {
    element_left_held = ElementPtr();
  }
}

void Gui::event_handling_middle_click(const MouseEvent& event) {
  if (event.action == MouseAction::Press) {
    element_middle_held = get_leaf_node(event.position);
    if (element_middle_held) {
      systems.mouse_event(element_middle_held, event);
    }
    return;
  }
  if (!element_middle_held) {
    return;
  }
  systems.mouse_event(element_middle_held, event);

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
  systems.mouse_hover(element_hover, mouse_pos);
}

void Gui::event_handling_scroll(const ScrollEvent& event) {
  ElementPtr element = get_leaf_node(event.position);
  while (element) {
    if (systems.scroll_event(element, event)) {
      return;
    }
    element = element.parent();
  }
}

void Gui::change_tree_focus(ElementPtr from, ElementPtr to) {
  struct HashFunc {
    std::size_t operator()(const ElementPtr& element) const {
      return element.hash();
    }
  };
  std::unordered_set<ElementPtr, HashFunc> removed;
  std::unordered_set<ElementPtr, HashFunc> added;

  if (from) {
    from.state().focused = false;
    auto iter = from;
    while (iter) {
      removed.insert(iter);
      iter.state().in_focus_tree = false;
      iter = iter.parent();
    }
  }

  std::vector<Layer> focused_layers;
  if (to) {
    to.state().focused = true;
    auto iter = to;
    while (iter) {
      added.insert(iter);
      if (iter.state().content_mode == DisplayMode::Float) {
        focused_layers.push_back(Layer{iter, true});
      }
      if (iter.state().display_mode == DisplayMode::Float) {
        focused_layers.push_back(Layer{iter, false});
      }
      iter = iter.parent();
    }
  }

  if (from) {
    systems.focus_leave(from, true);
  }
  for (auto iter : removed) {
    if (!added.contains(iter)) {
      systems.focus_tree_leave(iter);
    }
  }
  if (to) {
    systems.focus_enter(to);
  }

  if (!focused_layers.empty()) {
    for (auto iter = focused_layers.begin(); iter != focused_layers.end();
         ++iter) {
      auto existing = layers.find(*iter);
      if (existing != layers.end()) {
        existing->second.z_order = next_z_order++;
      }
    }
    layers_ordered.clear();
    for (const auto& layer_pair : layers) {
      layers_ordered.insert(layer_pair);
    }
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
  } while (next && next != tree.root() &&
           next.state().display_mode != DisplayMode::Disabled);

  if (next == tree.root() &&
      tree.root().state().display_mode == DisplayMode::Disabled) {
    next = ElementPtr();
  }

  change_tree_focus(element_focus, next);
}

template <typename T>
requires std::is_base_of_v<Viewport, T>
T& Gui::viewport() {
  current.expect(Type::ViewportPtr, read_key());
  args_.apply(current);
  auto& viewport = current.viewport();
  if (!viewport.viewport) {
    viewport.viewport = std::make_unique<T>();
    viewport.viewport->init(theme, font_registry);
  }
  move_down();
  viewport.viewport->begin();
  T* ptr = dynamic_cast<T*>(viewport.viewport.get());
  assert(ptr);
  return *ptr;
}
template Canvas2d& Gui::viewport<Canvas2d>();
template Canvas3d& Gui::viewport<Canvas3d>();
#if 0
template Plotter& Gui::viewport<Plotter>();
#endif

} // namespace dgui

#include "datagui/gui.hpp"
#include <queue>
#include <sstream>
#include <stack>

namespace datagui {

using namespace std::placeholders;

Gui::Gui(const Window::Config& config) :
    window(config),
    series_system(res),
    text_box_system(res),
    text_input_system(res),
    button_system(res),
    dropdown_system(res),
    floating_system(res),
    checkbox_system(res) {

  series_system.register_type(tree, systems);
  text_box_system.register_type(tree, systems);
  text_input_system.register_type(tree, systems);
  button_system.register_type(tree, systems);
  dropdown_system.register_type(tree, systems);
  floating_system.register_type(tree, systems);
  checkbox_system.register_type(tree, systems);

  res.geometry_renderer.init();
  res.text_renderer.init();
}

bool Gui::running() const {
  return window.running();
}

// Series

bool Gui::series_begin(const Style& style) {
  auto element = tree.next(series_system.type());
  res.style_manager.push_temp(style);
  series_system.visit(element);
  res.style_manager.pop_temp();
  if (tree.down_if()) {
    res.style_manager.down();
    return true;
  }
  return false;
}

void Gui::series_begin_force(const Style& style) {
  auto element = tree.next(series_system.type());
  res.style_manager.push_temp(style);
  series_system.visit(element);
  res.style_manager.pop_temp();
  tree.down();
  res.style_manager.down();
}

void Gui::series_end() {
  res.style_manager.up();
  tree.up();
}

// Text box

void Gui::text_box(const std::string& text, const Style& style) {
  auto element = tree.next(text_box_system.type());
  res.style_manager.push_temp(style);
  text_box_system.visit(element, text);
  res.style_manager.pop_temp();
}

// Button

bool Gui::button(const std::string& text, const Style& style) {
  auto element = tree.next(button_system.type());
  res.style_manager.push_temp(style);
  auto result = button_system.visit(element, text);
  res.style_manager.pop_temp();
  return result;
}

// Text input

const std::string* Gui::text_input(
    const std::string& initial_text,
    const Style& style) {
  auto element = tree.next(text_input_system.type());
  res.style_manager.push_temp(style);
  auto result = text_input_system.visit(element, initial_text);
  res.style_manager.pop_temp();
  return result;
}

void Gui::text_input(const Variable<std::string>& text, const Style& style) {
  auto element = tree.next(text_input_system.type());
  res.style_manager.push_temp(style);
  text_input_system.visit(element, text);
  res.style_manager.pop_temp();
}

void Gui::text_input_write(const std::string& value, const Style& style) {
  auto element = tree.next(text_input_system.type());
  res.style_manager.push_temp(style);
  text_input_system.write(element, value);
  res.style_manager.pop_temp();
}

const std::string& Gui::text_input_read() {
  auto element = tree.next(text_input_system.type());
  return text_input_system.read(element);
}

// Checkbox

const bool* Gui::checkbox(const bool& initial_checked, const Style& style) {
  auto element = tree.next(checkbox_system.type());
  res.style_manager.push_temp(style);
  auto result = checkbox_system.visit(element, initial_checked);
  res.style_manager.pop_temp();
  return result;
}

void Gui::checkbox(const Variable<bool>& checked, const Style& style) {
  auto element = tree.next(checkbox_system.type());
  res.style_manager.push_temp(style);
  checkbox_system.visit(element, checked);
  res.style_manager.pop_temp();
}

void Gui::checkbox_write(bool value, const Style& style) {
  auto element = tree.next(checkbox_system.type());
  res.style_manager.push_temp(style);
  checkbox_system.write(element, value);
  res.style_manager.pop_temp();
}

bool Gui::checkbox_read() {
  auto element = tree.next(checkbox_system.type());
  return checkbox_system.read(element);
}

// Dropdown

const int* Gui::dropdown(
    const std::vector<std::string>& choices,
    int initial_choice,
    const Style& style) {
  auto element = tree.next(dropdown_system.type());
  res.style_manager.push_temp(style);
  auto result = dropdown_system.visit(element, choices, initial_choice);
  res.style_manager.pop_temp();
  return result;
}

void Gui::dropdown(
    const std::vector<std::string>& choices,
    const Variable<int>& choice,
    const Style& style) {
  auto element = tree.next(dropdown_system.type());
  res.style_manager.push_temp(style);
  dropdown_system.visit(element, choices, choice);
  res.style_manager.pop_temp();
}

void Gui::dropdown_write(
    const std::vector<std::string>& choices,
    int choice,
    const Style& style) {
  auto element = tree.next(dropdown_system.type());
  res.style_manager.push_temp(style);
  dropdown_system.write(element, choices, choice);
  res.style_manager.pop_temp();
}

int Gui::dropdown_read() {
  auto element = tree.next(dropdown_system.type());
  return dropdown_system.read(element);
}

// Floating

bool Gui::floating_begin(
    const Variable<bool>& open,
    const std::string& title,
    const Style& style) {
  if (!*open) {
    return false;
  }

  auto element = tree.next(floating_system.type(), "floating");
  res.style_manager.push_temp(style);
  floating_system.visit(element, open, title);
  res.style_manager.pop_temp();
  if (!*open) {
    return false;
  }
  if (tree.down_if()) {
    res.style_manager.down();
    return true;
  }
  return false;
}

void Gui::floating_end() {
  res.style_manager.up();
  tree.up();
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

  struct State {
    ConstElement element;
    bool first_visit;
    State(ConstElement element) : element(element), first_visit(true) {}
  };
  std::stack<State> layer_stack;
  layer_stack.emplace(tree.root());

  struct Compare {
    bool operator()(const ConstElement& lhs, const ConstElement& rhs) {
      return lhs->layer >= rhs->layer;
    }
  };
  std::priority_queue<ConstElement, std::vector<ConstElement>, Compare>
      queued_elements;
  int current_layer = layer_stack.top().element->layer;

  while (true) {
    if (layer_stack.empty()) {
      res.geometry_renderer.render(window.size());
      res.text_renderer.render(window.size());
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
      res.geometry_renderer.pop_mask();
      res.text_renderer.pop_mask();
      layer_stack.pop();
      continue;
    }
    state.first_visit = false;

    if (!element.visible()) {
      continue;
    }
    systems.render(element);

    if (debug_mode_) {
      res.geometry_renderer.queue_box(
          Boxf(element->position, element->position + element->size),
          Color::Clear(),
          2,
          element->focused         ? Color::Blue()
          : element->in_focus_tree ? Color::Red()
                                   : Color::Green(),
          0);

      if (element->floating) {
        res.geometry_renderer.queue_box(
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
        auto text_size =
            res.font_manager.text_size(debug_text, text_style, LengthWrap());

        res.geometry_renderer.queue_box(
            Boxf(
                window.size() - text_size - Vecf::Constant(15),
                window.size() - Vecf::Constant(5)),
            Color::White(),
            2,
            Color::Black(),
            0);
        res.text_renderer.queue_text(
            window.size() - text_size - Vecf::Constant(10),
            debug_text,
            text_style,
            LengthWrap());
      }
    }

    if (element->floating) {
      res.geometry_renderer.push_mask(element->float_box);
      res.text_renderer.push_mask(element->float_box);
    } else {
      res.geometry_renderer.push_mask(element->box());
      res.text_renderer.push_mask(element->box());
    }

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
        for (auto child = element.first_child(); child; child = child.next()) {
          stack.emplace(child);
        }
        continue;
      }
      stack.pop();

      systems.set_input_state(element);
    }
  }

  {
    struct Compare {
      bool operator()(const Element& lhs, const Element& rhs) const {
        return lhs->float_priority > rhs->float_priority;
      }
    };
    struct Layer {
      ConstElement root;
      std::priority_queue<Element, std::vector<Element>, Compare> floating;
      Layer(ConstElement root) : root(root) {}
    };

    std::stack<Layer> layers;
    std::stack<Element> stack;
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

        Element next_floating = layer.floating.top();
        next_floating->layer_box = layer.root->box();
        next_floating->layer = next_layer;
        next_layer++;
        stack.push(next_floating);
        layers.emplace(next_floating);

        layer.floating.pop();
      }

      auto element = stack.top();
      stack.pop();

      if (!element.visible()) {
        continue;
      }

      if (element->floating && element != layers.top().root) {
        if (element.is_new()) {
          element->float_priority = next_float_priority++;
        }
        layers.top().floating.push(element);
        continue;
      }

      systems.set_dependent_state(element);

      for (auto child = element.first_child(); child; child = child.next()) {
        child->layer_box = element->layer_box;
        child->layer = element->layer;
        stack.push(child);
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
  int leaf_layer = -1;

  std::stack<Element> stack;
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

void Gui::set_tree_focus(Element element, bool focused) {
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

void GuiWriter::number(datapack::NumberType type, const void* value) {
  std::string value_str;
  switch (type) {
  case datapack::NumberType::I32:
    value_str = std::to_string(*(std::int32_t*)value);
    break;
  case datapack::NumberType::I64:
    value_str = std::to_string(*(std::int64_t*)value);
    break;
  case datapack::NumberType::U32:
    value_str = std::to_string(*(std::uint32_t*)value);
    break;
  case datapack::NumberType::U64:
    value_str = std::to_string(*(std::uint64_t*)value);
    break;
  case datapack::NumberType::U8:
    value_str = std::to_string(*(std::uint8_t*)value);
    break;
  case datapack::NumberType::F32:
    value_str = std::to_string(*(float*)value);
    break;
  case datapack::NumberType::F64:
    value_str = std::to_string(*(double*)value);
    break;
  }

  gui.text_input_write(value_str);
}

void GuiWriter::boolean(bool value) {
  gui.checkbox_write(value);
}

void GuiWriter::string(const char* value) {
  std::string value_str(value);
  gui.text_input_write(value_str);
}

void GuiWriter::enumerate(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }
  gui.dropdown_write(labels_str, value);
}

void GuiWriter::binary(const std::span<const std::uint8_t>& data) {
  gui.text_input("<binary not editable>");
}

void GuiWriter::optional_begin(bool has_value) {
  gui.variable<bool>(has_value);
  gui.checkbox_write(has_value);
  if (has_value) {
    gui.series_begin_force();
  }
}

void GuiWriter::optional_end() {
  gui.series_end();
}

void GuiWriter::variant_begin(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }
  gui.dropdown_write(labels_str, value);
  gui.series_begin_force();
}
void GuiWriter::variant_end() {
  gui.series_end();
}

void GuiWriter::object_begin() {
  gui.series_begin_force();
}
void GuiWriter::object_next(const char* key) {
  gui.text_box(key);
}
void GuiWriter::object_end() {
  gui.series_end();
}

void GuiWriter::tuple_begin() {
  gui.series_begin_force();
}
void GuiWriter::tuple_next() {
  // Do nothing
}
void GuiWriter::tuple_end() {
  gui.series_end();
}

void GuiWriter::list_begin() {
  gui.series_begin_force();
  list_sizes.push(gui.variable<std::size_t>(0));
}
void GuiWriter::list_next() {
  list_sizes.top().mut()++;
}
void GuiWriter::list_end() {
  gui.button("Push");
  gui.button("Pop");
  gui.series_end();
}

void GuiReader::number(datapack::NumberType type, void* value) {
  auto value_str = gui.text_input_read();
  try {
    switch (type) {
    case datapack::NumberType::I32:
      *(std::int32_t*)value = std::stoi(value_str);
      break;
    case datapack::NumberType::I64:
      *(std::int64_t*)value = std::stol(value_str);
      break;
    case datapack::NumberType::U32:
      *(std::uint32_t*)value = std::stoul(value_str);
      break;
    case datapack::NumberType::U64:
      *(std::uint64_t*)value = std::stoul(value_str);
      break;
    case datapack::NumberType::U8:
      *(std::uint8_t*)value = std::stoul(value_str);
      break;
    case datapack::NumberType::F32:
      *(float*)value = std::stof(value_str);
      break;
    case datapack::NumberType::F64:
      *(double*)value = std::stod(value_str);
      break;
    }
  } catch (const std::invalid_argument& e) {
    // TODO: Handle a better way
  }
}

bool GuiReader::boolean() {
  return gui.checkbox_read();
}

const char* GuiReader::string() {
  return gui.text_input_read().c_str();
}

int GuiReader::enumerate(const std::span<const char*>& labels) {
  return gui.dropdown_read();
}

std::span<const std::uint8_t> GuiReader::binary() {
  gui.text_box("Binary not implemented");
  return std::span<const std::uint8_t>((const std::uint8_t*)nullptr, 0);
}

bool GuiReader::optional_begin() {
  gui.series_begin_force();
  auto has_value = gui.variable<bool>();
  gui.checkbox(has_value);
  if (*has_value) {
    gui.series_begin_force();
    return true;
  }
  return false;
}

void GuiReader::optional_end() {
  gui.series_end();
}

int GuiReader::variant_begin(const std::span<const char*>& labels) {
  int value = gui.dropdown_read();
  gui.series_begin_force();
  return value;
}
void GuiReader::variant_end() {
  gui.series_end();
}

void GuiReader::object_begin() {
  gui.series_begin();
}
void GuiReader::object_next(const char* key) {
  gui.text_box(key);
}
void GuiReader::object_end() {
  gui.series_end();
}

void GuiReader::tuple_begin() {
  gui.series_begin();
}
void GuiReader::tuple_next() {
  // Do nothing
}
void GuiReader::tuple_end() {
  gui.series_end();
}

void GuiReader::list_begin() {
  gui.series_begin();
  auto size = gui.variable<std::size_t>(0);
  list_sizes.push(std::make_pair(size, *size));
}
bool GuiReader::list_next() {
  auto& [variable, remaining] = list_sizes.top();
  if (remaining == 0) {
    return false;
  }
  remaining--;
  return true;
}
void GuiReader::list_end() {
  auto size = list_sizes.top().first;
  if (gui.button("Push")) {
    size.set(*size + 1);
  }
  if (gui.button("Pop") && *size > 0) {
    size.set(*size - 1);
  }
  list_sizes.pop();
  gui.series_end();
}

} // namespace datagui

#include "datagui/datapack/reader.hpp"
#include "datagui/datapack/common.hpp"
#include <datapack/encode/base64.hpp>

namespace dgui {

namespace {

// Reads a number from the text input, following the same logic as
// Gui::number_input, with an initial value of zero.
// Returns true if the value changed.
template <typename T>
bool read_number(TextInput& text_input, bool is_new, void* value) {
  T& output = *(T*)value;

  if (is_new) {
    output = T(0);
    text_input.text = number_to_string(output);
    text_input.changed = false;
    return false;
  }

  bool changed = text_input.changed;
  text_input.changed = false;

  T number;
  if (text_to_number(text_input.text, number)) {
    output = number;
    return changed;
  }

  // Not a valid number, revert to the previous value
  text_input.text = number_to_string(output);
  return false;
}

} // namespace

void GuiReader::number(dpack::NumberType type, void* value) {
  if (in_color) {
    switch (type) {
      case dpack::NumberType::I32:
        *(std::int32_t*)value = color.data[color_i - 1] * 255;
        break;
      case dpack::NumberType::I64:
        *(std::int64_t*)value = color.data[color_i - 1] * 255;
        break;
      case dpack::NumberType::U32:
        *(std::int32_t*)value = color.data[color_i - 1] * 255;
        break;
      case dpack::NumberType::U64:
        *(std::uint64_t*)value = color.data[color_i - 1] * 255;
        break;
      case dpack::NumberType::F32:
        *(float*)value = color.data[color_i - 1];
        break;
      case dpack::NumberType::F64:
        *(double*)value = color.data[color_i - 1];
        break;
      case dpack::NumberType::U8:
        *(std::uint8_t*)value = color.data[color_i - 1] * 255;
        break;
    }
    return;
  }

  enter_primitive();

  auto hint = consume_hint();
  auto hint_range = hint ? std::get_if<dpack::HintRange>(&*hint) : nullptr;
  if (hint_range) {
    node.expect(Type::Slider, read_id());
    auto& slider = node.slider();
    slider.lower = hint_range->lower;
    slider.upper = hint_range->upper;
    slider.type = convert_type(type);

    changed_ |= slider.changed;
    slider.changed = false;

    switch (type) {
      case dpack::NumberType::I32:
        *(std::int32_t*)value = slider.value;
        break;
      case dpack::NumberType::I64:
        *(std::int64_t*)value = slider.value;
        break;
      case dpack::NumberType::U32:
        *(std::uint32_t*)value = slider.value;
        break;
      case dpack::NumberType::U64:
        *(std::uint64_t*)value = slider.value;
        break;
      case dpack::NumberType::U8:
        *(std::uint8_t*)value = slider.value;
        break;
      case dpack::NumberType::F32:
        *(float*)value = slider.value;
        break;
      case dpack::NumberType::F64:
        *(double*)value = slider.value;
        break;
    }
    return;
  }

  bool is_new = node.expect(Type::TextInput, read_id());
  auto& text_input = node.text_input();
  text_input.number_type = convert_type(type);

  switch (type) {
    case dpack::NumberType::I32:
      changed_ |= read_number<std::int32_t>(text_input, is_new, value);
      break;
    case dpack::NumberType::I64:
      changed_ |= read_number<std::int64_t>(text_input, is_new, value);
      break;
    case dpack::NumberType::U32:
      changed_ |= read_number<std::uint32_t>(text_input, is_new, value);
      break;
    case dpack::NumberType::U64:
      changed_ |= read_number<std::uint64_t>(text_input, is_new, value);
      break;
    case dpack::NumberType::U8:
      changed_ |= read_number<std::uint8_t>(text_input, is_new, value);
      break;
    case dpack::NumberType::F32:
      changed_ |= read_number<float>(text_input, is_new, value);
      break;
    case dpack::NumberType::F64:
      changed_ |= read_number<double>(text_input, is_new, value);
      break;
  }
}

bool GuiReader::boolean() {
  enter_primitive();
  node.expect(Type::Checkbox, read_id());
  auto& checkbox = node.checkbox();
  changed_ |= checkbox.changed;
  checkbox.changed = false;
  return checkbox.checked;
}

const char* GuiReader::string() {
  enter_primitive();
  node.expect(Type::TextInput, read_id());
  auto& text_input = node.text_input();
  changed_ |= text_input.changed;
  text_input.changed = false;
  return text_input.text.c_str();
}

int GuiReader::enumerate(const std::span<const char*>& labels) {
  enter_primitive();
  node.expect(Type::Select, read_id());
  auto& select = node.select();
  changed_ |= select.changed;
  select.changed = false;
  return select.choice;
}

std::span<const std::uint8_t> GuiReader::binary() {
  enter_primitive();
  node.expect(Type::TextInput, read_id());
  auto& text_input = node.text_input();
  changed_ |= text_input.changed;
  text_input.changed = false;
  try {
    binary_temp = dpack::base64_decode(text_input.text);
  } catch (const dpack::Base64Exception&) {
    // TODO: Handle text input constraints internally
    binary_temp.clear();
  }
  return binary_temp;
}

bool GuiReader::optional_begin() {
  enter_container(-1, 1);

  node.expect(Type::Group);
  {
    auto& group = node.group();
    group.layout.rows = 1;
    group.layout.cols = 2;
    group.layout.tight = true;
  }
  node = node.child();

  node.expect(Type::TextBox);
  node.text_box().text = "Has value?";
  node = node.next();

  bool has_value;
  node.expect(Type::Checkbox);
  {
    auto& checkbox = node.checkbox();
    changed_ |= checkbox.changed;
    checkbox.changed = false;
    has_value = checkbox.checked;
  }

  node = node.parent().next();
  if (node) {
    node.state().hidden = !has_value;
  }

  if (!has_value) {
    node = node.parent();
    return false;
  }
  in_composite_ = true;
  return true;
}

void GuiReader::optional_end() {
  in_composite_ = false;
  assert(!node.next());
  node = node.parent();
}

int GuiReader::variant_begin(const std::span<const char*>& labels) {
  enter_container(-1, 1);

  bool is_new = node.expect(Type::Select, read_id());
  auto& select = node.select();
  changed_ |= select.changed;
  select.changed = false;
  select.choices.clear();
  for (auto label : labels) {
    node.select().choices.emplace_back(label);
  }
  if (is_new) {
    // Always set default value to 0
    select.choice = 0;
  }

  int choice = select.choice;
  // Offset by one, so that choice 0 doesn't collide with the unset id
  std::size_t expected_id = 1 + choice;

  node = node.next();
  while (node && node.id() != expected_id) {
    node.state().hidden = true;
    node = node.next();
  }
  next_id_ = expected_id;
  if (node) {
    node.state().hidden = false;
  }

  in_composite_ = true;
  return choice;
}

void GuiReader::variant_end() {
  in_composite_ = false;
  if (node) {
    node = node.next();
    while (node) {
      node.state().hidden = true;
      node = node.next();
    }
  }
  node = node.parent();
}

void GuiReader::object_begin() {
  auto hint = consume_hint();
  auto hint_color = hint ? std::get_if<dpack::HintColor>(&*hint) : nullptr;
  if (hint_color) {
    enter_primitive();
    node.expect(Type::ColorPicker, read_id());
    auto& color_picker = node.color_picker();
    changed_ |= color_picker.changed;
    color_picker.changed = false;
    in_color = true;
    color = color_picker.value;
    color_i = 0;
    return;
  }

  enter_container(-1, 2);
  at_object_begin = true;
}

void GuiReader::object_next(const char* key) {
  if (in_color) {
    assert(color_i < 4);
    color_i++;
    return;
  }
  if (!at_object_begin) {
    node = node.next();
  }
  at_object_begin = false;
  next_label_ = key;
}

void GuiReader::object_end() {
  if (in_color) {
    assert(color_i == 4);
    in_color = false;
    return;
  }

  if (at_object_begin) {
    assert(!node);
  } else {
    assert(!node.next());
  }
  at_object_begin = false;

  node = node.parent();
}

void GuiReader::tuple_begin() {
  enter_container(-1, 1);
  at_object_begin = true;
}

void GuiReader::tuple_next() {
  if (!at_object_begin) {
    node = node.next();
  }
  at_object_begin = false;
  next_label_ = "";
}

void GuiReader::tuple_end() {
  if (at_object_begin) {
    assert(!node);
  } else {
    assert(!node.next());
  }
  at_object_begin = false;

  node = node.parent();
}

size_t GuiReader::list_begin() {
  enter_container(2, 1);
  // Contains:
  // <items group>
  // <add button>

  auto var = node.parent().var();
  if (!var) {
    var.create(ListVar());
  }
  list_stack.emplace(var.as<ListVar>());
  auto& list_state = list_stack.top();

  if (list_state.var->dirty) {
    // An item was added or removed on the previous cycle
    changed_ = true;
  }

  // Enter items group
  node.expect(Type::Group);
  auto& group = node.group();
  group.layout.tight = true;
  group.layout.rows = -1;
  group.layout.cols = 1;
  node = node.child();

  at_object_begin = true;
  return list_state.var->ids.size();
}

void GuiReader::list_next() {
  if (!at_object_begin) {
    node = node.next();
    list_item_end();
  }
  at_object_begin = false;

  list_item_begin();
}

void GuiReader::list_end() {
  auto& list_state = list_stack.top();

  if (!at_object_begin) {
    node = node.next();
    list_item_end();
    assert(list_state.pos == list_state.var->ids.size());
  }

  node.expect_end();
  node = node.parent();
  at_object_begin = false;

  node = node.next();
  node.expect(Type::Button);
  auto& button = node.button();
  button.text = "Add";
  if (button.released) {
    // Set changed = true on the next poll()
    button.released = false;
    list_state.current_dirty = true;
    list_state.var->ids.append();
  }

  list_state.var->dirty = list_state.current_dirty;
  list_stack.pop();

  assert(!node.next());
  node = node.parent();
}

void GuiReader::list_item_begin() {
  auto& list_state = list_stack.top();

  // The value of state.var->ids.size() should inform the external datapack code
  // how many times to call list_next()
  // If this isn't followed, then it will go out of bounds here
  assert(list_state.pos < list_state.var->ids.size());

  // Any items removed on the previous cycle are erased by expect() searching
  // forwards for this id
  node.expect(Type::Group, list_state.var->ids[list_state.pos]);
  auto& group = node.group();
  group.layout.tight = true;
  group.layout.rows = 1;
  group.layout.cols = 3;
  node = node.child();

  next_label_ = "[" + std::to_string(list_state.pos) + "]";
}

void GuiReader::list_item_end() {
  auto& list_state = list_stack.top();

  node.expect(Type::Button);
  auto& button = node.button();
  button.text = "Remove";
  const bool removed = button.released;

  if (removed) {
    // Set changed = true on the next poll()
    button.released = false;
    list_state.var->ids.remove(list_state.var->ids[list_state.pos]);
    list_state.current_dirty = true;
  }

  node = node.next();
  node.expect_end();
  node = node.parent();

  if (!removed) {
    list_state.pos++;
    node = node.next();
  }
}

void GuiReader::enter_primitive() {
  if (is_root_) {
    is_root_ = false;
    node.expect(Type::Group);
    root = node;
    node.group().layout.tight = true;
    node = node.child();
    return;
  }

  // No special handling for composites, only used by enter_container
  in_composite_ = false;
  if (!next_label_.empty()) {
    node.expect(Type::TextBox);
    node.text_box().text = next_label_;
    node = node.next();
  }
  next_label_.clear();
}

void GuiReader::enter_container(size_t rows, size_t cols) {
  if (in_composite_) {
    assert(!is_root_);
    in_composite_ = false;
    node.expect(Type::Group, read_id());
    auto& group = node.group();
    group.layout.tight = true;
    group.layout.rows = rows;
    group.layout.cols = cols;
    node = node.child();
    return;
  }

  node.expect(Type::Collapsable, read_id());
  auto& collapsable = node.collapsable();
  collapsable.label = next_label_;
  collapsable.layout.rows = rows;
  collapsable.layout.cols = cols;

  if (!next_label_.empty() && !is_root_) {
    node.state().num_cells = 2;
  }
  next_label_.clear();
  if (is_root_) {
    root = node;
    is_root_ = false;
  }
  node = node.child();
}

bool GuiReader::peek_changed(ConstElementPtr root) {
  if (!root) {
    return false;
  }

  std::stack<ConstElementPtr> stack;
  stack.push(root);
  while (!stack.empty()) {
    auto node = stack.top();

    switch (node.type()) {
      case Type::Button:
        if (node.button().released) {
          return true;
        }
        break;
      case Type::Checkbox:
        if (node.checkbox().changed) {
          return true;
        }
        break;
      case Type::ColorPicker:
        if (node.color_picker().changed) {
          return true;
        }
        break;
      case Type::Select:
        if (node.select().changed) {
          return true;
        }
        break;
      case Type::Slider:
        if (node.slider().changed) {
          return true;
        }
        break;
      case Type::TextInput:
        if (node.text_input().changed) {
          return true;
        }
        break;
      default:
        break;
    }

    if (node.var()) {
      auto list_var = node.var().as<ListVar>();
      if (list_var && list_var->dirty) {
        return true;
      }
    }

    stack.pop();
    auto child = node.child();
    while (child) {
      stack.push(child);
      child = child.next();
    }
  }
  return false;
}

} // namespace dgui

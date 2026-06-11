#include "datagui/datapack/writer.hpp"
#include "datagui/datapack/common.hpp"
#include <array>
#include <charconv>
#include <datapack/encode/base64.hpp>

namespace datagui {

template <typename T>
std::string number_to_string(T value) {
  std::array<char, 64> buffer;
  auto result =
      std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
  if (result.ec != std::errc{}) {
    return "0";
  }
  return std::string(buffer.data(), result.ptr);
}

void GuiWriter::number(dpack::NumberType type, const void* value) {
  if (in_color) {
    float& output = color.data[color_i - 1];
    switch (type) {
    case dpack::NumberType::I32:
      output = double(*(const std::int32_t*)value) / 255;
      break;
    case dpack::NumberType::I64:
      output = double(*(const std::int64_t*)value) / 255;
      break;
    case dpack::NumberType::U32:
      output = double(*(const std::uint32_t*)value) / 255;
      break;
    case dpack::NumberType::U64:
      output = double(*(const std::uint64_t*)value) / 255;
      break;
    case dpack::NumberType::U8:
      output = double(*(const std::uint8_t*)value) / 255;
      break;
    case dpack::NumberType::F32:
      output = *(const float*)value;
      break;
    case dpack::NumberType::F64:
      output = *(const double*)value;
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

    switch (type) {
    case dpack::NumberType::I32:
      slider.value = *(const std::int32_t*)value;
      break;
    case dpack::NumberType::I64:
      slider.value = *(const std::int64_t*)value;
      break;
    case dpack::NumberType::U32:
      slider.value = *(const std::uint32_t*)value;
      break;
    case dpack::NumberType::U64:
      slider.value = *(const std::uint64_t*)value;
      break;
    case dpack::NumberType::U8:
      slider.value = *(const std::uint8_t*)value;
      break;
    case dpack::NumberType::F32:
      slider.value = *(const float*)value;
      break;
    case dpack::NumberType::F64:
      slider.value = *(const double*)value;
      break;
    }
    slider.changed = false;
    return;
  }

  node.expect(Type::TextInput, read_id());
  auto& text_input = node.text_input();

  switch (type) {
  case dpack::NumberType::I32:
    text_input.text = number_to_string(*(const std::int32_t*)value);
    break;
  case dpack::NumberType::I64:
    text_input.text = number_to_string(*(const std::int64_t*)value);
    break;
  case dpack::NumberType::U32:
    text_input.text = number_to_string(*(const std::uint32_t*)value);
    break;
  case dpack::NumberType::U64:
    text_input.text = number_to_string(*(const std::uint64_t*)value);
    break;
  case dpack::NumberType::U8:
    text_input.text = number_to_string(*(const std::uint8_t*)value);
    break;
  case dpack::NumberType::F32:
    text_input.text = number_to_string(*(const float*)value);
    break;
  case dpack::NumberType::F64:
    text_input.text = number_to_string(*(const double*)value);
    break;
  }
  text_input.changed = false;
}

void GuiWriter::boolean(bool value) {
  enter_primitive();
  node.expect(Type::Checkbox, read_id());
  auto& checkbox = node.checkbox();
  checkbox.checked = value;
  checkbox.changed = false;
}

void GuiWriter::string(const char* value) {
  enter_primitive();
  node.expect(Type::TextInput, read_id());
  auto& text_input = node.text_input();
  text_input.text = value;
  text_input.changed = false;
}

void GuiWriter::enumerate(int value, const std::span<const char*>& labels) {
  enter_primitive();
  node.expect(Type::Select, read_id());
  auto& select = node.select();
  select.choices.clear();
  for (auto label : labels) {
    select.choices.emplace_back(label);
  }
  select.choice = value;
  select.changed = false;
}

void GuiWriter::binary(const std::span<const std::uint8_t>& data) {
  enter_primitive();
  node.expect(Type::TextInput, read_id());
  auto& text_input = node.text_input();
  text_input.text = dpack::base64_encode(data);
  text_input.changed = false;
}

void GuiWriter::optional_begin(bool has_value) {
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

  node.expect(Type::Checkbox);
  {
    auto& checkbox = node.checkbox();
    checkbox.checked = has_value;
    checkbox.changed = false;
  }

  node = node.parent().next();
  if (node) {
    node.state().force_hidden = !has_value;
  }

  if (!has_value) {
    // optional_end() is not called when there is no value
    node = node.parent();
    return;
  }
  in_composite_ = true;
}

void GuiWriter::optional_end() {
  in_composite_ = false;
  assert(!node.next());
  node = node.parent();
}

void GuiWriter::variant_begin(
    int value,
    const std::span<const char*>& labels) {
  enter_container(-1, 1);

  node.expect(Type::Select, read_id());
  auto& select = node.select();
  select.choices.clear();
  for (auto label : labels) {
    select.choices.emplace_back(label);
  }
  select.choice = value;
  select.changed = false;

  node = node.next();
  while (node && node.id() != value) {
    node.state().force_hidden = true;
    node = node.next();
  }
  if (node) {
    node.state().force_hidden = false;
  }
  next_id_ = value;

  in_composite_ = true;
}

void GuiWriter::variant_end() {
  in_composite_ = false;
  if (node) {
    node = node.next();
    while (node) {
      node.state().force_hidden = true;
      node = node.next();
    }
  }
  node = node.parent();
}

void GuiWriter::object_begin() {
  auto hint = consume_hint();
  auto hint_color = hint ? std::get_if<dpack::HintColor>(&*hint) : nullptr;
  if (hint_color) {
    enter_primitive();
    node.expect(Type::ColorPicker, read_id());
    in_color = true;
    color = Color::Black();
    color_i = 0;
    return;
  }

  enter_container(-1, 2);
  at_object_begin = true;
}

void GuiWriter::object_next(const char* key) {
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

void GuiWriter::object_end() {
  if (in_color) {
    assert(color_i == 4);
    auto& color_picker = node.color_picker();
    color_picker.value = color;
    color_picker.changed = false;
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

void GuiWriter::tuple_begin() {
  enter_container(-1, 1);
  at_object_begin = true;
}

void GuiWriter::tuple_next() {
  if (!at_object_begin) {
    node = node.next();
  }
  at_object_begin = false;
  next_label_ = "";
}

void GuiWriter::tuple_end() {
  if (at_object_begin) {
    assert(!node);
  } else {
    assert(!node.next());
  }
  at_object_begin = false;

  node = node.parent();
}

void GuiWriter::list_begin(size_t size) {
  enter_container(2, 1);
  // Contains:
  // <items group>
  // <add button>

  auto var = node.parent().var();
  if (!var) {
    var.create(ListVar());
  }
  auto list_var = var.as<ListVar>();

  // Resize the key list to exactly match the value being written, reusing
  // existing keys (and therefore widgets) where possible.
  while (list_var->ids.size() < size) {
    list_var->ids.append();
  }
  while (list_var->ids.size() > size) {
    list_var->ids.remove(list_var->ids[list_var->ids.size() - 1]);
  }
  // Writing overwrites widget state authoritatively, so there is no pending
  // add/remove to reconcile.
  list_var->dirty = false;

  list_stack.emplace(list_var);

  // Enter items group
  node.expect(Type::Group);
  auto& group = node.group();
  group.layout.tight = true;
  group.layout.rows = -1;
  group.layout.cols = 3;
  node = node.child();

  at_object_begin = true;
}

void GuiWriter::list_next() {
  auto& list_state = list_stack.top();

  if (!at_object_begin) {
    node = node.next();
    list_remove_button();
    list_state.pos++;
    node = node.next();
  }
  at_object_begin = false;

  list_item_label();
  node = node.next();
}

void GuiWriter::list_end() {
  auto& list_state = list_stack.top();

  if (!at_object_begin) {
    node = node.next();
    list_remove_button();
    list_state.pos++;
    assert(list_state.pos == list_state.var->ids.size());
    node = node.next();
  }

  while (node) {
    // Skip the Label, value, remove button
    node = node.erase().erase().erase();
  }
  at_object_begin = false;

  node = node.parent().next();
  node.expect(Type::Button);
  node.button().text = "Add";

  list_stack.pop();
  assert(!node.next());
  node = node.parent();
}

void GuiWriter::list_item_label() {
  auto& list_state = list_stack.top();

  std::uint64_t expected_id = list_state.var->ids[list_state.pos];
  while (node && node.id() != expected_id) {
    // Skip the Label, value, remove button
    node = node.erase().erase().erase();
  }

  node.expect(Type::TextBox, expected_id);
  node.text_box().text = "Item " + std::to_string(list_state.pos);
}

void GuiWriter::list_remove_button() {
  node.expect(Type::Button);
  node.button().text = "Remove";
}

void GuiWriter::enter_primitive() {
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

void GuiWriter::enter_container(size_t rows, size_t cols) {
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
  if (is_root_) {
    is_root_ = false;
    root = node;
  }
  next_label_.clear();
  node = node.child();
}

} // namespace datagui

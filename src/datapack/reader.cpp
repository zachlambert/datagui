#include "datagui/datapack/reader.hpp"
#include <charconv>
#include <datapack/encode/base64.hpp>

namespace datagui {

template <typename T>
T number_from_string(const std::string& string) {
  T value;
  auto error =
      std::from_chars(string.data(), string.data() + string.size(), value).ec;
  if (error != std::errc{}) {
    return T(0);
  }
  return value;
}

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

  node.expect(Type::TextInput, read_id());
  auto& text_input = node.text_input();
  changed_ |= text_input.changed;
  text_input.changed = false;

  switch (type) {
  case dpack::NumberType::I32:
    *(std::int32_t*)value = number_from_string<std::int32_t>(text_input.text);
    break;
  case dpack::NumberType::I64:
    *(std::int64_t*)value = number_from_string<std::int64_t>(text_input.text);
    break;
  case dpack::NumberType::U32:
    *(std::uint32_t*)value = number_from_string<std::uint32_t>(text_input.text);
    break;
  case dpack::NumberType::U64:
    *(std::uint64_t*)value = number_from_string<std::uint64_t>(text_input.text);
    break;
  case dpack::NumberType::U8:
    *(std::uint8_t*)value = number_from_string<std::uint8_t>(text_input.text);
    break;
  case dpack::NumberType::F32:
    *(float*)value = number_from_string<float>(text_input.text);
    break;
  case dpack::NumberType::F64:
    *(double*)value = number_from_string<double>(text_input.text);
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
    node.state().force_hidden = !has_value;
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

  node.expect(Type::Select, read_id());
  auto& select = node.select();
  changed_ |= select.changed;
  select.changed = false;
  select.choices.clear();
  for (auto label : labels) {
    node.select().choices.emplace_back(label);
  }

  int choice = select.choice;
  node = node.next();
  while (node && node.id() != choice) {
    node.state().force_hidden = true;
    node = node.next();
  }
  if (node) {
    node.state().force_hidden = false;
  }
  next_id_ = choice;

  in_composite_ = true;
  return choice;
}

void GuiReader::variant_end() {
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

void GuiReader::object_begin() {
  auto hint = consume_hint();
  auto hint_color = hint ? std::get_if<dpack::HintColor>(&*hint) : nullptr;
  if (hint_color) {
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
  enter_container(-1, 2);

  auto var = node.parent().var();
  if (!var) {
    var.create(KeyList());
  }
  list_stack.emplace(var.as<KeyList>());

  at_object_begin = true;

  return list_stack.top().keys->size();
}

static constexpr std::uint64_t add_button_id =
    std::numeric_limits<std::uint64_t>::max();

void GuiReader::list_next() {
  if (!at_object_begin) {
    node = node.next();
    if (!list_remove_button()) {
      list_stack.top().pos++;
    }
    node = node.next();
  }
  at_object_begin = false;

  auto& state = list_stack.top();
  std::uint64_t expected_id = (*state.keys)[state.pos];
  while (node && node.id() != expected_id && node.id() != add_button_id) {
    auto remove_button = node.next();
    auto next = remove_button.next();
    node.erase();
    remove_button.erase();
    node = next;
  }
  if (node.id() == add_button_id) {
    // Remove add button, re-add in list_end()
    node.erase();
  }

  next_id_ = expected_id;
}

void GuiReader::list_end() {
  if (!at_object_begin) {
    node = node.next();
    list_remove_button();
    node = node.next();
  }

  while (node && node.id() != add_button_id) {
    auto remove_button = node.next();
    auto next = remove_button.next();
    node.erase();
    remove_button.erase();
    node = next;
  }
  at_object_begin = false;

  node.expect(Type::Button, add_button_id);
  auto& button = node.button();
  button.text = "Add";
  if (button.released) {
    button.released = false;
    auto& state = list_stack.top();
    list_stack.top().keys->append();
  }

  assert(!node.next());
  node = node.parent();
}

bool GuiReader::list_remove_button() {
  node.expect(Type::Button);
  auto& button = node.button();
  button.text = "Remove";
  if (button.released) {
    button.released = false;
    auto& state = list_stack.top();
    state.keys->remove((*state.keys)[state.pos]);
    return true;
  }
  return false;
}

void GuiReader::enter_primitive() {
  in_composite_ = false;
  is_root_ = false;

  if (!next_label_.empty()) {
    node.expect(Type::TextBox);
    node.text_box().text = next_label_;
    node = node.next();
  }
  next_label_.clear();
}

void GuiReader::enter_container(size_t rows, size_t cols) {
  if (in_composite_) {
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
  is_root_ = false;
  node = node.child();
}

ElementPtr GuiReader::next_node() {
  return node.next();
}

} // namespace datagui

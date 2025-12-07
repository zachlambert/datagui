#include "datagui/datapack/writer.hpp"
#include "datagui/gui.hpp"
#include <datapack/encode/base64.hpp>

namespace datagui {

void GuiWriter::number(datapack::NumberType type, const void* value) {
  if (in_color) {
    float& output = color.data[color_i - 1];
    std::string value_str;
    switch (type) {
    case datapack::NumberType::I32:
      output = double(*(std::int32_t*)value) / 255;
      break;
    case datapack::NumberType::I64:
      output = double(*(std::int64_t*)value) / 255;
      break;
    case datapack::NumberType::U32:
      output = double(*(std::uint32_t*)value) / 255;
      break;
    case datapack::NumberType::U64:
      output = double(*(std::uint64_t*)value) / 255;
      break;
    case datapack::NumberType::U8:
      output = double(*(std::uint8_t*)value) / 255;
      break;
    case datapack::NumberType::F32:
      output = *(float*)value;
      break;
    case datapack::NumberType::F64:
      output = *(double*)value;
      break;
    }
    return;
  }
  if (auto constraint = get_constraint<datapack::ConstraintNumber>()) {
    if (auto range =
            std::get_if<datapack::ConstraintNumberRange>(&(*constraint))) {

      switch (type) {
      case datapack::NumberType::I32:
        gui.slider<std::int32_t>(
            *(std::int32_t*)value,
            range->lower,
            range->upper,
            {});
        break;
      case datapack::NumberType::I64:
        gui.slider<std::int64_t>(
            *(std::int64_t*)value,
            range->lower,
            range->upper,
            {});
        break;
      case datapack::NumberType::U32:
        gui.slider<std::uint32_t>(
            *(std::uint32_t*)value,
            range->lower,
            range->upper,
            {});
        break;
      case datapack::NumberType::U64:
        gui.slider<std::uint64_t>(
            *(std::uint64_t*)value,
            range->lower,
            range->upper,
            {});
        break;
      case datapack::NumberType::F32:
        gui.slider<float>(*(float*)value, range->lower, range->upper, {});
        break;
      case datapack::NumberType::F64:
        gui.slider<double>(*(double*)value, range->lower, range->upper, {});
        break;
      case datapack::NumberType::U8:
        gui.slider<std::uint8_t>(
            *(std::uint8_t*)value,
            range->lower,
            range->upper,
            {});
        break;
      }
    }
  }

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

  gui.text_input(value_str, {});
}

void GuiWriter::boolean(bool value) {
  gui.checkbox(value, {});
}

void GuiWriter::string(const char* value) {
  gui.text_input(std::string(value), {});
}

void GuiWriter::enumerate(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }
  gui.select(labels_str, value, {});
}

void GuiWriter::binary(const std::span<const std::uint8_t>& data) {
  std::string text = datapack::base64_encode(data);
  gui.text_input(text, {});
}

void GuiWriter::optional_begin(bool has_value) {
  gui.args().tight();
  gui.group();
  auto has_value_var = gui.variable<bool>(has_value);
  gui.checkbox(has_value_var);
  if (!*has_value_var) {
    gui.end();
  }
}

void GuiWriter::optional_end() {
  gui.end();
}

void GuiWriter::variant_begin(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }

  gui.args().tight();
  gui.group();
  auto choice_var = gui.variable<int>(value);
  gui.select(labels_str, choice_var);
  gui.key(*choice_var);
}

void GuiWriter::variant_end() {
  gui.end();
}

void GuiWriter::object_begin() {
  if (auto constraint = get_constraint<datapack::ConstraintObject>()) {
    if (std::get_if<datapack::ConstraintObjectColor>(&(*constraint))) {
      in_color = true;
      color = Color::Black();
      color_i = 0;
      return;
    }
  }
  gui.args().tight();
  gui.group();
  at_object_begin = true;
}

void GuiWriter::object_next(const char* key) {
  if (in_color) {
    assert(color_i < 4);
    color_i++;
    return;
  }
  if (!at_object_begin) {
    gui.end();
  }
  at_object_begin = false;
  gui.collapsable(key);
}

void GuiWriter::object_end() {
  if (in_color) {
    assert(color_i == 4);
    gui.color_picker(color, {});
    in_color = false;
    return;
  }
  if (!at_object_begin) {
    gui.end();
  }
  at_object_begin = false;
  gui.end();
}

void GuiWriter::tuple_begin() {
  gui.args().tight();
  gui.group();
}

void GuiWriter::tuple_next() {
  // Do nothing
}

void GuiWriter::tuple_end() {
  gui.end();
}

void GuiWriter::list_begin() {
  gui.args().tight();
  gui.group();

  auto key_list = gui.variable<KeyList>();
  ListState state = {key_list, 0};
  list_stack.push(state);

  gui.args().tight();
  gui.group();
}

void GuiWriter::list_next() {
  assert(!list_stack.empty());
  auto& state = list_stack.top();

  if (state.index != 0) {
    auto keys = state.keys;
    std::size_t key = (*keys)[state.index - 1];
    gui.button("Remove", [=]() { keys.mut().remove(key); });
    gui.end();
  }

  assert(state.index == state.keys->size());
  std::size_t new_key = state.keys.mut_internal().append();

  gui.key((*state.keys)[state.index]);
  gui.args().horizontal();
  gui.group();

  state.index++;
}

void GuiWriter::list_end() {
  assert(!list_stack.empty());
  const auto& state = list_stack.top();
  auto keys = state.keys;

  if (state.index != 0) {
    std::size_t remove_i = state.index - 1;
    gui.button("Remove", [=]() { keys.mut().remove(remove_i); });
    gui.end();
  }

  gui.end();

  gui.button("new", [=]() { keys.mut().append(); });
  gui.end();

  list_stack.pop();
}

} // namespace datagui

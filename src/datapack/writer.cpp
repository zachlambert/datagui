#include "datagui/datapack/writer.hpp"

#include "datagui/element/button.hpp"
#include "datagui/element/checkbox.hpp"
#include "datagui/element/dropdown.hpp"
#include "datagui/element/series.hpp"
#include "datagui/element/text_box.hpp"
#include "datagui/element/text_input.hpp"

namespace datagui {

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

  auto& props = get_text_input(systems, *tree.next(), "0");
  props.text = value_str;
}

void GuiWriter::boolean(bool value) {
  auto& props = get_checkbox(systems, *tree.next(), false);
  props.checked = value;
}

void GuiWriter::string(const char* value) {
  auto& props = get_text_input(systems, *tree.next(), "");
  props.text = value;
}

void GuiWriter::enumerate(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }

  auto& props = get_dropdown(systems, *tree.next(), labels_str, -1);
  props.choices = labels_str;
  props.choice = value;
}

void GuiWriter::binary(const std::span<const std::uint8_t>& data) {
  auto& props = get_text_box(systems, *tree.next());
  props.text = "<binary not editable>";
}

void GuiWriter::optional_begin(bool has_value) {
  get_series(systems, *tree.next());

  DATAGUI_LOG("GuiWriter::optional_begin", "DOWN (1)");
  tree.down();

  auto& toggle = get_checkbox(systems, *tree.next(), false);
  toggle.checked = has_value;

  get_series(systems, *tree.next());

  if (!has_value) {
    DATAGUI_LOG("GuiWriter::optional_begin", "UP (1) (no value)");
    tree.up();
    return;
  }

  DATAGUI_LOG("GuiWriter::optional_begin", "DOWN (2) (has value)");
  tree.down();
}

void GuiWriter::optional_end() {
  DATAGUI_LOG("GuiWriter::object_end", "UP (2)");
  tree.up();

  DATAGUI_LOG("GuiWriter::object_end", "UP (1)");
  tree.up();
}

void GuiWriter::variant_begin(int value, const std::span<const char*>& labels) {
  get_series(systems, *tree.next());

  tree.down();
  DATAGUI_LOG("GuiWriter::variant_begin", "DOWN (1)");

  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }

  auto& dropdown = get_dropdown(systems, *tree.next(), labels_str, -1);
  dropdown.choices = labels_str;
  dropdown.choice = value;

  auto id_var = tree.variable<int>([&]() { return 0; });
  id_var.mutate(value);

  get_series(systems, *tree.next(value));

  tree.down();
  DATAGUI_LOG("GuiWriter::variant_begin", "DOWN (2)");
}

void GuiWriter::variant_end() {
  DATAGUI_LOG("GuiWriter::variant_end", "UP (2)");
  tree.up();
  DATAGUI_LOG("GuiWriter::variant_end", "UP (2)");
  tree.up();
}

void GuiWriter::object_begin() {
  get_series(systems, *tree.next());

  DATAGUI_LOG("GuiWriter::object_begin", "DOWN");
  tree.down();
}

void GuiWriter::object_next(const char* key) {
  auto& props = get_text_box(systems, *tree.next());
  props.text = key;
}

void GuiWriter::object_end() {
  DATAGUI_LOG("GuiWriter::object_end", "UP");
  tree.up();
}

void GuiWriter::tuple_begin() {
  get_series(systems, *tree.next());

  DATAGUI_LOG("GuiWriter::tuple_begin", "DOWN");
  tree.down();
}

void GuiWriter::tuple_next() {
  // Do nothing
}

void GuiWriter::tuple_end() {
  DATAGUI_LOG("GuiWriter::tuple_end", "UP");
  tree.up();
}

void GuiWriter::list_begin() {
  get_series(systems, *tree.next());
  DATAGUI_LOG("GuiWriter::list_begin", "DOWN (1)");
  tree.down();

  ListState state;
  state.ids_var =
      tree.variable<std::vector<int>>([]() { return std::vector<int>(); });
  list_stack.push(state);

  get_series(systems, *tree.next());
  DATAGUI_LOG("GuiWriter::list_begin", "DOWN (2)");
  tree.down();
}

void GuiWriter::list_next() {
  auto& state = list_stack.top();

  if (state.index != 0) {
    DATAGUI_LOG("GuiWriter::list_next", "UP (el %zu)", state.index - 1);
    tree.up();
  }

  int element_id = tree.get_id();
  state.ids.push_back(element_id);

  DATAGUI_LOG(
      "GuiWriter::list_next",
      "ELEMENT: index=%zu, id=%i",
      state.index,
      element_id);

  get_series(systems, *tree.next(element_id));
  DATAGUI_LOG("GuiWriter::list_next", "DOWN (el %zu)", state.index);
  state.index++;

  tree.down();
}

void GuiWriter::list_end() {
  auto& state = list_stack.top();
  state.ids_var.mutate(state.ids);
  if (state.index != 0) {
    DATAGUI_LOG("GuiWriter::list_end", "UP (el %zu)", state.index - 1);
    tree.up();
  }

  DATAGUI_LOG("GuiWriter::list_end", "UP (2)");
  tree.up();

  auto& push_button = get_button(systems, *tree.next());
  push_button.text = "Push";

  auto& pop_button = get_button(systems, *tree.next());
  pop_button.text = "Pop";

  DATAGUI_LOG("GuiWriter::list_end", "UP (1)");
  tree.up();
}

} // namespace datagui

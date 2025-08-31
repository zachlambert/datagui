#include "datagui/datapack/reader.hpp"

#include "datagui/element/button.hpp"
#include "datagui/element/checkbox.hpp"
#include "datagui/element/dropdown.hpp"
#include "datagui/element/floating.hpp"
#include "datagui/element/series.hpp"
#include "datagui/element/text_box.hpp"
#include "datagui/element/text_input.hpp"

namespace datagui {

void GuiReader::number(datapack::NumberType type, void* value) {
  const auto& props = get_text_input(systems, *tree.next(), "0");

  try {
    switch (type) {
    case datapack::NumberType::I32:
      *(std::int32_t*)value = std::stoi(props.text);
      break;
    case datapack::NumberType::I64:
      *(std::int64_t*)value = std::stol(props.text);
      break;
    case datapack::NumberType::U32:
      *(std::uint32_t*)value = std::stoul(props.text);
      break;
    case datapack::NumberType::U64:
      *(std::uint64_t*)value = std::stoul(props.text);
      break;
    case datapack::NumberType::U8:
      *(std::uint8_t*)value = std::stoul(props.text);
      break;
    case datapack::NumberType::F32:
      *(float*)value = std::stof(props.text);
      break;
    case datapack::NumberType::F64:
      *(double*)value = std::stod(props.text);
      break;
    }
  } catch (const std::invalid_argument& e) {
    // TODO: Handle a better way
  }
}

bool GuiReader::boolean() {
  const auto& props = get_checkbox(systems, *tree.next(), false);
  return props.checked;
}

const char* GuiReader::string() {
  const auto& props = get_text_input(systems, *tree.next(), "");
  return props.text.c_str();
}

int GuiReader::enumerate(const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }

  const auto& props = get_dropdown(systems, *tree.next(), labels_str, 0);
  return props.choice;
}

std::span<const std::uint8_t> GuiReader::binary() {
  auto& props = get_text_box(systems, *tree.next());
  props.text = "<binary not editable>";
  return std::span<const std::uint8_t>((const std::uint8_t*)nullptr, 0);
}

bool GuiReader::optional_begin() {
  get_series(systems, *tree.next());

  tree.down();
  DATAGUI_LOG("GuiReader::optional_begin", "DOWN (1)");

  const auto& toggle = get_checkbox(systems, *tree.next(), false);

  if (!toggle.checked) {
    DATAGUI_LOG("GuiReader::optional_begin", "UP (1) (no value)");
    tree.up();
    return false;
  }

  get_series(systems, *tree.next());

  DATAGUI_LOG("GuiReader::optional_begin", "DOWN (1) (has value)");
  tree.down();
  return true;
}

void GuiReader::optional_end() {
  DATAGUI_LOG("GuiReader::optional_end", "UP (2)");
  tree.up();
  DATAGUI_LOG("GuiReader::optional_end", "UP (1)");
  tree.up();
}

int GuiReader::variant_begin(const std::span<const char*>& labels) {
  get_series(systems, *tree.next());

  DATAGUI_LOG("GuiReader::variant_begin", "DOWN (1)");
  tree.down();

  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }

  bool changed = false;
  auto& dropdown = get_dropdown(systems, *tree.next(), labels_str, 0);
  if (dropdown.changed) {
    dropdown.changed = false;
  }

  auto id_var = tree.variable<int>([&]() { return tree.get_id(); });
  int id = *id_var;
  if (dropdown.changed) {
    id = tree.get_id();
    id_var.set(id);
  }

  get_series(systems, *tree.next(id));

  DATAGUI_LOG("GuiReader::variant_begin", "DOWN (2)");
  tree.down();

  return dropdown.choice;
}

void GuiReader::variant_end() {
  DATAGUI_LOG("GuiReader::variant_end", "UP (2)");
  tree.up();
  DATAGUI_LOG("GuiReader::variant_end", "UP (1)");
  tree.up();
}

void GuiReader::object_begin() {
  get_series(systems, *tree.next());
  DATAGUI_LOG("GuiReader::object_begin", "DOWN");
  tree.down();
}

void GuiReader::object_next(const char* key) {
  auto& props = get_text_box(systems, *tree.next());
  props.text = key;
}

void GuiReader::object_end() {
  DATAGUI_LOG("GuiReader::object_begin", "UP");
  tree.up();
}

void GuiReader::tuple_begin() {
  get_series(systems, *tree.next());
  DATAGUI_LOG("GuiReader::tuple_begin", "DOWN");
  tree.down();
}

void GuiReader::tuple_next() {
  // Do nothing
}

void GuiReader::tuple_end() {
  DATAGUI_LOG("GuiReader::tuple_end", "UP");
  tree.up();
}

void GuiReader::list_begin() {
  get_series(systems, *tree.next());
  DATAGUI_LOG("GuiReader::list_begin", "DOWN (1)");
  tree.down();

  ListState state;
  state.ids_var =
      tree.variable<std::vector<int>>([]() { return std::vector<int>(); });
  state.index = 0;

  get_series(systems, *tree.next());
  DATAGUI_LOG("GuiReader::list_begin", "DOWN (2)");
  tree.down();
}

bool GuiReader::list_next() {
  auto& state = list_stack.top();
  if (state.index == state.ids_var->size()) {
    return false;
  }
  state.index++;
  return true;
}

void GuiReader::list_end() {
  DATAGUI_LOG("GuiReader::list_end", "UP (2)");
  tree.up();

  const auto& state = list_stack.top();

  auto& push_button = get_button(systems, *tree.next());
  push_button.text = "Push";
  if (push_button.released) {
    push_button.released = false;
    auto new_ids = *state.ids_var;
    new_ids.push_back(tree.get_id());
    state.ids_var.set(new_ids);
  }

  auto& pop_button = get_button(systems, *tree.next());
  pop_button.text = "Pop";
  if (pop_button.released) {
    pop_button.released = false;
    if (!state.ids_var->empty()) {
      auto new_ids = *state.ids_var;
      new_ids.pop_back();
      state.ids_var.set(new_ids);
    }
  }

  list_stack.pop();

  DATAGUI_LOG("GuiReader::list_end", "UP (1)");
  tree.up();
}

} // namespace datagui

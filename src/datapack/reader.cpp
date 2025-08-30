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
  auto element = tree.next();
  auto props = element->props.cast<TextInputProps>();
  assert(props);

  try {
    switch (type) {
    case datapack::NumberType::I32:
      *(std::int32_t*)value = std::stoi(props->text);
      break;
    case datapack::NumberType::I64:
      *(std::int64_t*)value = std::stol(props->text);
      break;
    case datapack::NumberType::U32:
      *(std::uint32_t*)value = std::stoul(props->text);
      break;
    case datapack::NumberType::U64:
      *(std::uint64_t*)value = std::stoul(props->text);
      break;
    case datapack::NumberType::U8:
      *(std::uint8_t*)value = std::stoul(props->text);
      break;
    case datapack::NumberType::F32:
      *(float*)value = std::stof(props->text);
      break;
    case datapack::NumberType::F64:
      *(double*)value = std::stod(props->text);
      break;
    }
  } catch (const std::invalid_argument& e) {
    // TODO: Handle a better way
  }
}

bool GuiReader::boolean() {
  auto element = tree.next();
  auto props = element->props.cast<CheckboxProps>();
  assert(props);
  return props->checked;
}

const char* GuiReader::string() {
  auto element = tree.next();
  auto props = element->props.cast<TextInputProps>();
  assert(props);
  return props->text.c_str();
}

int GuiReader::enumerate(const std::span<const char*>& labels) {
  auto element = tree.next();
  auto props = element->props.cast<DropdownProps>();
  assert(props);
  return props->choice;
}

std::span<const std::uint8_t> GuiReader::binary() {
  auto element = tree.next();
  auto props = element->props.cast<TextBoxProps>();
  assert(props);
  // No change
  return std::span<const std::uint8_t>((const std::uint8_t*)nullptr, 0);
}

bool GuiReader::optional_begin() {
  {
    auto element = tree.next();
    assert(element->props.cast<SeriesProps>());
  }

  tree.down();
  DATAGUI_LOG("GuiReader::optional_begin", "DOWN (1)");

  bool has_value;
  {
    auto element = tree.next();
    auto props = element->props.cast<CheckboxProps>();
    assert(props);
    has_value = props->checked;
  }

  if (!has_value) {
    DATAGUI_LOG("GuiReader::optional_begin", "UP (1) (no value)");
    tree.up();
    return false;
  }

  {
    auto element = tree.next();
    assert(element->props.cast<SeriesProps>());
  }

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
  {
    auto element = tree.next();
    assert(element->props.cast<SeriesProps>());
  }

  DATAGUI_LOG("GuiReader::variant_begin", "DOWN (1)");
  tree.down();

  int value;
  {
    auto element = tree.next();
    auto props = element->props.cast<DropdownProps>();
    assert(props);
    value = props->choice;
  }

  {
    auto element = tree.next();
    assert(element->props.cast<SeriesProps>());
  }

  DATAGUI_LOG("GuiReader::variant_begin", "DOWN (2)");
  tree.down();

  return value;
}

void GuiReader::variant_end() {
  DATAGUI_LOG("GuiReader::variant_end", "UP (2)");
  tree.up();
  DATAGUI_LOG("GuiReader::variant_end", "UP (1)");
  tree.up();
}

void GuiReader::object_begin() {
  auto element = tree.next();
  assert(element->props.cast<SeriesProps>());
  DATAGUI_LOG("GuiReader::object_begin", "DOWN");
  tree.down();
}

void GuiReader::object_next(const char* key) {
  auto element = tree.next();
  auto props = element->props.cast<TextBoxProps>();
  assert(props);
  assert(props->text == key);
}

void GuiReader::object_end() {
  DATAGUI_LOG("GuiReader::object_begin", "UP");
  tree.up();
}

void GuiReader::tuple_begin() {
  auto element = tree.next();
  assert(element->props.cast<SeriesProps>());
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
  {
    auto element = tree.next();
    assert(element->props.cast<SeriesProps>());
  }
  DATAGUI_LOG("GuiReader::list_begin", "DOWN (1)");
  tree.down();

  ListState state;
  state.ids_var =
      tree.variable<std::vector<int>>([]() { return std::vector<int>(); });
  state.index = 0;

  {
    auto element = tree.next();
    assert(element->props.cast<SeriesProps>());
  }
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

  {
    auto element = tree.next();
    auto props = element->props.cast<ButtonProps>();
    assert(props);
    assert(props->text == "Push");

    if (props->released) {
      props->released = false;
      auto new_ids = *state.ids_var;
      new_ids.push_back(tree.get_id());
      state.ids_var.set(new_ids);
    }
  }

  {
    auto element = tree.next();
    auto props = element->props.cast<ButtonProps>();
    assert(props);
    assert(props->text == "Pop");

    if (props->released) {
      props->released = false;
      auto new_ids = *state.ids_var;
      if (new_ids.size() > 0) {
        new_ids.pop_back();
        state.ids_var.set(new_ids);
      }
    }
  }

  list_stack.pop();

  DATAGUI_LOG("GuiReader::list_end", "UP (1)");
  tree.up();
}

} // namespace datagui

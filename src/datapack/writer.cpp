#include "datagui/datapack/writer.hpp"

#include "datagui/element/button.hpp"
#include "datagui/element/checkbox.hpp"
#include "datagui/element/dropdown.hpp"
#include "datagui/element/floating.hpp"
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

  auto element = tree.next();
  if (element->system == -1) {
    DATAGUI_LOG("[GuiWriter::number] Construct new TextInput");
    element->props = UniqueAny::Make<TextInputProps>();
    element->system = systems.find<TextInputSystem>();
  }

  auto& props = *element->props.cast<TextInputProps>();
  props.text = value_str;
  props.set_style(*sm);
}

void GuiWriter::boolean(bool value) {
  auto element = tree.next();
  if (element->system == -1) {
    DATAGUI_LOG("[GuiWriter::boolean] Construct new Checkbox");
    element->props = UniqueAny::Make<CheckboxProps>();
    element->system = systems.find<CheckboxSystem>();
  }

  auto& props = *element->props.cast<CheckboxProps>();
  props.checked = value;
  props.set_style(*sm);
}

void GuiWriter::string(const char* value) {
  auto element = tree.next();
  if (element->system == -1) {
    DATAGUI_LOG("[GuiWriter::string] Construct new TextInput");
    element->props = UniqueAny::Make<TextInputProps>();
    element->system = systems.find<TextInputSystem>();
  }

  auto& props = *element->props.cast<TextInputProps>();
  props.text = value;
  props.set_style(*sm);
}

void GuiWriter::enumerate(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }

  auto element = tree.next();
  if (element->system == -1) {
    DATAGUI_LOG("[GuiWriter::enumerate] Construct new Dropdown");
    element->props = UniqueAny::Make<DropdownProps>();
    element->system = systems.find<DropdownSystem>();
  }

  auto& props = *element->props.cast<DropdownProps>();
  props.choices = labels_str;
  props.choice = value;
  props.set_style(*sm);
}

void GuiWriter::binary(const std::span<const std::uint8_t>& data) {
  auto element = tree.next();
  if (element->system == -1) {
    DATAGUI_LOG("[GuiWriter::binary] Construct new TextBox");
    element->props = UniqueAny::Make<TextBoxProps>();
    element->system = systems.find<TextBoxSystem>();
  }

  auto& props = *element->props.cast<TextBoxProps>();
  props.text = "<binary not editable>";
  props.set_style(*sm);
}

void GuiWriter::optional_begin(bool has_value) {
  {
    auto element = tree.next();
    if (element->system == -1) {
      DATAGUI_LOG("[GuiWriter::optional_begin] Construct new Series (1)");
      element->props = UniqueAny::Make<SeriesProps>();
      element->system = systems.find<SeriesSystem>();
    }
    auto& props = *element->props.cast<SeriesProps>();
    props.set_style(*sm);
  }

  DATAGUI_LOG("[GuiWriter::optional_begin] DOWN (1)");
  tree.down();

  {
    auto element = tree.next();
    if (element->system == -1) {
      DATAGUI_LOG("[GuiWriter::optional_begin] Construct new Checkbox");
      element->props = UniqueAny::Make<CheckboxProps>();
      element->system = systems.find<CheckboxSystem>();
    }
    auto& props = *element->props.cast<CheckboxProps>();
    props.checked = has_value;
    props.set_style(*sm);
  }

  {
    auto element = tree.next();
    if (element->system == -1) {
      DATAGUI_LOG("[GuiWriter::optional_begin] Construct new Series (2)");
      element->props = UniqueAny::Make<SeriesProps>();
      element->system = systems.find<SeriesSystem>();
    }
    auto& props = *element->props.cast<SeriesProps>();
    props.set_style(*sm);
  }

  if (!has_value) {
    DATAGUI_LOG("[GuiWriter::optional_begin] UP (1) (no value)");
    tree.up();
    return;
  }

  DATAGUI_LOG("[GuiWriter::optional_begin] DOWN (2) (has value)");
  tree.down();
}

void GuiWriter::optional_end() {
  DATAGUI_LOG("[GuiWriter::object_end] UP (2)");
  tree.up();

  DATAGUI_LOG("[GuiWriter::object_end] UP (1)");
  tree.up();
}

void GuiWriter::variant_begin(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }

  {
    auto element = tree.next();
    if (element->system == -1) {
      DATAGUI_LOG("[GuiWriter::variant_begin] Construct new Series (1)");
      element->props = UniqueAny::Make<SeriesProps>();
      element->system = systems.find<SeriesSystem>();
    }
    auto& props = *element->props.cast<SeriesProps>();
    props.set_style(*sm);
  }

  tree.down();
  DATAGUI_LOG("[GuiWriter::variant_begin] DOWN (1)");

  {
    auto element = tree.next();
    if (element->system == -1) {
      DATAGUI_LOG("[GuiWriter::variant_begin] Construct new Dropdown");
      element->props = UniqueAny::Make<DropdownProps>();
      element->system = systems.find<DropdownSystem>();
    }

    auto& props = *element->props.cast<DropdownProps>();
    props.choices = labels_str;
    props.choice = value;
    props.set_style(*sm);
  }

  {
    auto element = tree.next();
    if (element->system == -1) {
      DATAGUI_LOG("[GuiWriter::variant_begin] Construct new Series (2)");
      element->props = UniqueAny::Make<SeriesProps>();
      element->system = systems.find<SeriesSystem>();
    }
    auto& props = *element->props.cast<SeriesProps>();
    props.set_style(*sm);
  }

  tree.down();
  DATAGUI_LOG("[GuiWriter::variant_begin] DOWN (2)");
}

void GuiWriter::variant_end() {
  DATAGUI_LOG("[GuiWriter::variant_end] UP (2)");
  tree.up();
  DATAGUI_LOG("[GuiWriter::variant_end] UP (2)");
  tree.up();
}

void GuiWriter::object_begin() {
  auto element = tree.next();
  if (element->system == -1) {
    DATAGUI_LOG("[GuiWriter::object_begin] Construct new Series");
    element->props = UniqueAny::Make<SeriesProps>();
    element->system = systems.find<SeriesSystem>();
  }
  auto& props = *element->props.cast<SeriesProps>();
  props.set_style(*sm);

  DATAGUI_LOG("[GuiWriter::object_begin] DOWN");
  tree.down();
}

void GuiWriter::object_next(const char* key) {
  auto element = tree.next();
  if (element->system == -1) {
    DATAGUI_LOG("[GuiWriter::object_next] Construct new TextBox (%s)", key);
    element->props = UniqueAny::Make<TextBoxProps>();
    element->system = systems.find<TextBoxSystem>();
  }
  auto& props = *element->props.cast<TextBoxProps>();
  props.set_style(*sm);
  props.text = key;
}

void GuiWriter::object_end() {
  DATAGUI_LOG("[GuiWriter::object_end] UP");
  tree.up();
}

void GuiWriter::tuple_begin() {
  auto element = tree.next();
  if (element->system == -1) {
    DATAGUI_LOG("[GuiWriter::tuple_begin] Construct new Series");
    element->props = UniqueAny::Make<SeriesProps>();
    element->system = systems.find<SeriesSystem>();
  }
  auto& props = *element->props.cast<SeriesProps>();
  props.set_style(*sm);

  DATAGUI_LOG("[GuiWriter::tuple_begin] DOWN");
  tree.down();
}

void GuiWriter::tuple_next() {
  // Do nothing
}

void GuiWriter::tuple_end() {
  DATAGUI_LOG("[GuiWriter::tuple_end] UP");
  tree.up();
}

void GuiWriter::list_begin() {
  {
    auto element = tree.next();
    if (element->system == -1) {
      DATAGUI_LOG("[GuiWriter::list_begin] Construct new Series (1)");
      element->props = UniqueAny::Make<SeriesProps>();
      element->system = systems.find<SeriesSystem>();
    }
    auto& props = *element->props.cast<SeriesProps>();
    props.set_style(*sm);
  }

  DATAGUI_LOG("[GuiWriter::list_begin] DOWN (1)");
  tree.down();

  ListState state;
  state.ids_var =
      tree.variable<std::vector<int>>([]() { return std::vector<int>(); });
  list_stack.push(state);

  {
    auto element = tree.next();
    if (element->system == -1) {
      DATAGUI_LOG("[GuiWriter::list_begin] Construct new Series (2)");
      element->props = UniqueAny::Make<SeriesProps>();
      element->system = systems.find<SeriesSystem>();
    }
    auto& props = *element->props.cast<SeriesProps>();
    props.set_style(*sm);
  }

  DATAGUI_LOG("[GuiWriter::list_begin] DOWN (2)");
  tree.down();
}

void GuiWriter::list_next() {
  list_stack.top().ids.push_back(tree.get_id());
}

void GuiWriter::list_end() {
  list_stack.top().ids_var.set(list_stack.top().ids);

  DATAGUI_LOG("[GuiWriter::list_end] UP (2)");
  tree.up();

  {
    auto element = tree.next();
    if (element->system == -1) {
      DATAGUI_LOG("[GuiWriter::list_end] Construct new Button (Push)");
      element->props = UniqueAny::Make<ButtonProps>();
      element->system = systems.find<ButtonSystem>();
    }
    auto& props = *element->props.cast<ButtonProps>();
    props.set_style(*sm);
    props.text = "Push";
  }

  {
    auto element = tree.next();
    if (element->system == -1) {
      DATAGUI_LOG("[GuiWriter::list_end] Construct new Button (Pop)");
      element->props = UniqueAny::Make<ButtonProps>();
      element->system = systems.find<ButtonSystem>();
    }
    auto& props = *element->props.cast<ButtonProps>();
    props.set_style(*sm);
    props.text = "Pop";
  }

  DATAGUI_LOG("[GuiWriter::list_end] UP (1)");
  tree.up();
}

} // namespace datagui

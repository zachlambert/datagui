#pragma once

#include "datagui/element/key_list.hpp"
#include "datagui/element/tree.hpp"
#include <datapack/datapack.hpp>
#include <stack>

namespace datagui {

class Gui;
class GuiWriter : public datapack::Writer {
public:
  GuiWriter(Gui& gui, const std::string& root_label) :
      gui(gui), next_label(root_label) {}

  void number(datapack::NumberType type, const void* value) override;
  void boolean(bool value) override;
  void string(const char*) override;
  void enumerate(int value, const std::span<const char*>& labels) override;
  void binary(const std::span<const std::uint8_t>& data) override;

  void optional_begin(bool has_value) override;
  void optional_end() override;

  void variant_begin(int value, const std::span<const char*>& labels) override;
  void variant_end() override;

  void object_begin() override;
  void object_next(const char* key) override;
  void object_end() override;

  void tuple_begin() override;
  void tuple_next() override;
  void tuple_end() override;

  void list_begin() override;
  void list_next() override;
  void list_end() override;

private:
  Gui& gui;

  void make_label();
  void make_collapsable();
  std::string next_label;
  bool label_from_object = false;

  struct ListState {
    Var<KeyList> keys;
    std::size_t index;
  };
  std::stack<ListState> list_stack;

  bool in_color = false;
  std::size_t color_i;
  Color color;
};

template <typename T>
T datapack_write(Gui& gui, const std::string& root_label, const T& value) {
  GuiWriter(gui, root_label).value(value);
  return value;
}

} // namespace datagui

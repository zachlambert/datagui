#pragma once

#include "datagui/tree/tree.hpp"
#include <datapack/datapack.hpp>

namespace datagui {

class GuiWriter : public datapack::Writer {
public:
  GuiWriter(ElementSystemList& systems, Tree& tree) :
      systems(systems), tree(tree) {}

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
  ElementSystemList& systems;
  Tree& tree;

  struct ListState {
    std::vector<int> ids;
    Variable<std::vector<int>> ids_var;
    std::size_t index = 0;
  };
  std::stack<ListState> list_stack;
  bool at_object_begin = false;
};

} // namespace datagui

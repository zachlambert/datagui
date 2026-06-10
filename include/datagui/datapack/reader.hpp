#pragma once

#include "datagui/element/tree.hpp"
#include <datapack/datapack.hpp>

namespace datagui {

class GuiReader : public dpack::Reader {
public:
  GuiReader(ConstElementPtr node) : node(node) {}

  void number(dpack::NumberType type, void* value) override;
  bool boolean() override;
  const char* string() override;
  int enumerate(const std::span<const char*>& labels) override;
  std::span<const std::uint8_t> binary() override;

  bool optional_begin() override;
  void optional_end() override;

  int variant_begin(const std::span<const char*>& labels) override;
  void variant_end() override;

  void object_begin() override;
  void object_next(const char* key) override;
  void object_end() override;

  void tuple_begin() override;
  void tuple_next() override;
  void tuple_end() override;

  size_t list_begin() override;
  void list_next() override;
  void list_end() override;

private:
  ConstElementPtr node;
  std::vector<std::uint8_t> binary_temp;
  bool at_object_begin = false;

  bool in_color = false;
  std::size_t color_i;
  Color color;
};

template <typename T>
void dpack_read(ConstElementPtr node, T& value) {
  GuiReader reader(node);
  reader.value(value);
  assert(reader.valid());
}

} // namespace datagui

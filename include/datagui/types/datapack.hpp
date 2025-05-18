#pragma once

#include "datagui/gui.hpp"
#include <datapack/datapack.hpp>

namespace datagui {

class GuiWriter : public datapack::Writer {
public:
  GuiWriter(Gui& gui) : gui(gui) {}

  void number(datapack::NumberType type, const void* value) override;
  void boolean(bool value) override;
  void string(const char*) override;
  void enumerate(int value, const char* label) override;
  void binary(const std::span<const std::uint8_t>& data) override;

  void optional_begin(bool has_value) override;
  void optional_end() override;

  void variant_begin(int value, const char* label) override;
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
};

class GuiReader : public datapack::Reader {
public:
  GuiReader(Gui& gui) : gui(gui) {}

  void number(datapack::NumberType type, void* value) override;
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

  void list_begin() override;
  bool list_next() override;
  void list_end() override;

private:
  Gui& gui;
};

} // namespace datagui

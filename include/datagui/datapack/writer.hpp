#pragma once

#include "datagui/datapack/common.hpp"
#include "datagui/element/tree.hpp"
#include <datapack/datapack.hpp>
#include <stack>

namespace datagui {

// Mirror of GuiReader, but copies data from a type into the gui structure,
// overwriting the current widget state, rather than reading the widget state
// back into the type.
class GuiWriter : public dpack::Writer {
public:
  GuiWriter(ElementPtr& root, const std::string& root_label) :
      root(root), node(root), next_label_(root_label) {}

  void number(dpack::NumberType type, const void* value) override;
  void boolean(bool value) override;
  void string(const char* value) override;
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

  void list_begin(size_t size) override;
  void list_next() override;
  void list_end() override;

  void hint(const dpack::Hint& hint) override {
    hint_ = hint;
  }
  void description(const std::string& description) override {
    description_ = description;
  }

private:
  void enter_primitive();
  void enter_container(size_t rows, size_t cols);
  void list_item_label();
  void list_remove_button();

  std::uint64_t read_id() {
    std::int64_t id = next_id_;
    next_id_ = 0;
    return id;
  }
  std::optional<dpack::Hint> consume_hint() {
    if (hint_) {
      return std::move(hint_);
    }
    return std::nullopt;
  }
  std::optional<std::string> consume_description() {
    if (description_) {
      return std::move(description_);
    }
    return std::nullopt;
  }

  ElementPtr& root;
  ElementPtr node;
  std::uint64_t next_id_ = 0;
  std::string next_label_ = "";
  bool is_root_ = true;
  bool in_composite_ = false;

  std::optional<dpack::Hint> hint_;
  std::optional<std::string> description_;

  struct ListState {
    Var<ListVar> var;
    int pos = 0;
    ListState(Var<ListVar> var) : var(var) {}
  };
  std::stack<ListState> list_stack;

  bool at_object_begin = false;

  bool in_color = false;
  std::size_t color_i;
  Color color;
};

} // namespace datagui

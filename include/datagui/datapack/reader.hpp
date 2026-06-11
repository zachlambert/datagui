#pragma once

#include "datagui/datapack/common.hpp"
#include "datagui/element/tree.hpp"
#include <datapack/datapack.hpp>
#include <stack>

namespace dgui {

class GuiReader : public dpack::Reader {
public:
  GuiReader(ElementPtr& root, const std::string& root_label) :
      root(root), node(root), next_label_(root_label) {}

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

  void hint(const dpack::Hint& hint) override {
    hint_ = hint;
  }
  void description(const std::string& description) override {
    description_ = description;
  }

  bool changed() const {
    return changed_;
  }

  static bool peek_changed(ConstElementPtr root);

private:
  void enter_primitive();
  void enter_container(size_t rows, size_t cols);
  void list_item_label();
  bool list_remove_button();

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

  bool changed_ = false;

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
    bool current_dirty = false;
    ListState(Var<ListVar> var) : var(var) {}
  };
  std::stack<ListState> list_stack;

  std::vector<std::uint8_t> binary_temp;
  bool at_object_begin = false;

  bool in_color = false;
  std::size_t color_i;
  Color color;
};

} // namespace dgui

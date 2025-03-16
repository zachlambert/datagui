#pragma once

#include "datagui/internal/element.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/vector_map.hpp"

namespace datagui {

struct Wrapper {
  std::string key;
  bool open;
  Wrapper(const std::string& key, bool open) : key(key), open(open) {}
};

class WrapperSystem : public ElementSystem {
public:
  WrapperSystem() {}

  int create(const std::string& key, bool open) {
    return elements.emplace(key, open);
  }

  void pop(int index) override {
    elements.pop(index);
  }

  bool update(const Node& node, const std::string& key, bool open);

  void calculate_size_components(Node& node, const Tree& tree) const override;

  void calculate_child_dimensions(const Node& node, Tree& tree) const override;

  void render(const Node& node, const NodeState& state, Renderers& renderers)
      const override { /* Do nothing */ }

private:
  VectorMap<Wrapper> elements;
};

} // namespace datagui

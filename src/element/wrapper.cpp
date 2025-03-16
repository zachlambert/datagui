#include "datagui/element/wrapper.hpp"
#include <assert.h>

namespace datagui {

bool WrapperSystem::update(const Node& node, const std::string& key, bool open) {
  auto& element = elements[node.element_index];
  bool changed = false;
  if (element.key != key) {
    element.key = key;
    changed = true;
  }
  if (element.open != open) {
    element.open = open;
    changed = true;
  }
  return changed;
}

void WrapperSystem::calculate_size_components(Node& node, const Tree& tree) const {

  node.fixed_size = Vecf::Zero();
  node.dynamic_size = Vecf::Zero();

  if (node.first_child == -1) {
    return;
  }

  // Expect only one child for wrapper
  const auto& child = tree[node.first_child];
  assert(child.next == -1);
  if (child.hidden) {
    return;
  }

  node.fixed_size = child.fixed_size;
  node.dynamic_size = child.dynamic_size;
}

void WrapperSystem::calculate_child_dimensions(const Node& node, Tree& tree) const {
  if (node.first_child == -1) {
    return;
  }

  auto& child = tree[node.first_child];
  assert(child.next == -1);

  child.origin = node.origin;
  child.size = node.size;
}

} // namespace datagui

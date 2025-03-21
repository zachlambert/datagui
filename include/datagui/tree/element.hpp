#pragma once

#include "datagui/tree/state.hpp"
#include "datagui/tree/vector_map.hpp"
#include "datagui/visual/renderers.hpp"
#include <concepts>
#include <functional>
#include <type_traits>

namespace datagui {

class ElementSystem {
public:
  virtual int emplace() = 0;
  virtual void pop(int index) = 0;
  virtual void render(const State& state, Renderers& renderers) const = 0;
};

template <typename Element>
requires std::is_default_constructible_v<Element>
class ElementSystemBase : public ElementSystem {
public:
  int emplace() override final {
    return elements.emplace();
  }
  void pop(int index) override final {
    elements.pop(index);
  }
  Element& operator[](int index) {
    return elements[index];
  }
  const Element& operator[](int index) const {
    return elements[index];
  }

private:
  VectorMap<Element> elements;
};

} // namespace datagui

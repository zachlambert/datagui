#pragma once

#include "datagui/color.hpp"
#include <variant>

namespace dgui {

class GradientMap {
    struct Linear {
      Color min;
      Color max;
    };
    struct Viridis {};
    std::variant<Linear, Viridis> impl_;
public:
  static GradientMap linear(Color min, Color max) {
    return GradientMap(Linear{min, max});
  }
  static GradientMap viridis() {
    return GradientMap(Viridis{});
  }

  Color lookup(float s) const;

private:
    template <class T>
        requires std::constructible_from<decltype(impl_), T>
    GradientMap(T&& t) : impl_(std::forward<T>(t)) {}
};

} // namespace dgui

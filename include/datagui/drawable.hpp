#pragma once

namespace dgui {

class Canvas2d;
class Canvas3d;

template <typename T>
struct DrawArgs {};

template <typename T>
concept drawable2d =
    requires(Canvas2d& canvas, const T& data, const DrawArgs<T>& args) {
      draw(canvas, data, args);
    };

template <typename T>
concept drawable3d =
    requires(Canvas3d& canvas, const T& data, const DrawArgs<T>& args) {
      draw(canvas, data, args);
    };

} // namespace dgui

#include <datagui/gui.hpp>
#include <datapack/debug.hpp>
#include <datapack/labelled_variant.hpp>

#include <datapack/std/array.hpp>
#include <datapack/std/optional.hpp>
#include <datapack/std/string.hpp>
#include <datapack/std/variant.hpp>
#include <datapack/std/vector.hpp>

struct Point {
  double x;
  double y;
};
struct Circle {
  double x;
  double y;
  double r;
};

using Shape = std::variant<Point, Circle>;

namespace dpack {

DPACK_INLINE(Point, x, y)
DPACK_INLINE(Circle, x, y, r)

DPACK_LABELLED_VARIANT(Shape, 2);
DPACK_LABELLED_VARIANT_DEF(Shape) = {"Point", "Circle"};

} // namespace dpack

int main() {
  dgui::Gui gui;

  Shape shape;

  int revisit = 0;
  while (gui.poll()) {
    gui.args().width_expand();
    gui.group();
    DGUI_SCOPE(gui);

    auto points = gui.variable<std::vector<Point>>();
    gui.edit("Points", points);

    auto shape = gui.variable<Shape>();
    gui.edit("Shape", shape);
  }
  return 0;
}

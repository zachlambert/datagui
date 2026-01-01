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

namespace datapack {

DATAPACK_INLINE(Point, value, packer) {
  packer.object_begin();
  packer.value("x", value.x);
  packer.value("y", value.y);
  packer.object_end();
}

DATAPACK_INLINE(Circle, value, packer) {
  packer.object_begin();
  packer.value("x", value.x);
  packer.value("y", value.y);
  packer.value("r", value.r);
  packer.object_end();
}

DATAPACK_LABELLED_VARIANT(Shape, 2);
DATAPACK_LABELLED_VARIANT_DEF(Shape) = {"Point", "Circle"};

} // namespace datapack

int main() {
  datagui::Gui gui;

  int revisit = 0;
  while (gui.running()) {
    gui.args().width_expand();
    if (gui.group()) {
      auto points = gui.variable<std::vector<Point>>();
      gui.edit("Points", points);

      auto shape = gui.variable<Shape>();
      gui.edit("Shape", shape);

      gui.end();
    }
    gui.poll();
  }
  return 0;
}

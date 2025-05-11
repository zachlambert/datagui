#include <datagui/gui.hpp>
#include <datapack/common.hpp>
#include <datapack/labelled_variant.hpp>
#include <datapack/util/debug.hpp>
#include <iostream>

struct Point {
  double x;
  double y;
};

struct Line {
  double x1;
  double y1;
  double x2;
  double y2;
};

using Shape = std::variant<Point, Line>;

struct Foo {
  int x;
  int y;
  bool test;
  Shape shape;
  std::optional<std::array<double, 3>> points;
  std::vector<std::string> names;
};

namespace datapack {

DATAPACK_LABELLED_VARIANT(Shape, 2);
DATAPACK_LABELLED_VARIANT_DEF(Shape) = {"point", "line"};

DATAPACK_INLINE(Point, value, packer) {
  packer.value("x", value.x);
  packer.value("y", value.x);
}

DATAPACK_INLINE(Line, value, packer) {
  packer.value("x1", value.x1);
  packer.value("y1", value.y1);
  packer.value("x2", value.x2);
  packer.value("y2", value.y2);
}

DATAPACK_INLINE(Foo, value, packer) {
  packer.object_begin();
  packer.value("x", value.x);
  packer.value("y", value.y);
  packer.value("test", value.test);
  packer.value("shape", value.shape);
  packer.value("points", value.points);
  packer.value("names", value.names);
  packer.object_end();
}
} // namespace datapack

int main() {
  datagui::Gui gui;
  Foo foo;

  while (gui.running()) {
    gui.begin();
    gui.end();
  }
  return 0;
}

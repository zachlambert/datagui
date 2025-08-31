#include <datagui/gui.hpp>
#include <datapack/debug.hpp>
#include <datapack/labelled_variant.hpp>
#include <iostream>
#include <optional>

#include <datapack/std/array.hpp>
#include <datapack/std/optional.hpp>
#include <datapack/std/string.hpp>
#include <datapack/std/variant.hpp>
#include <datapack/std/vector.hpp>

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
  packer.object_begin();
  packer.value("x", value.x);
  packer.value("y", value.x);
  packer.object_end();
}

DATAPACK_INLINE(Line, value, packer) {
  packer.object_begin();
  packer.value("x1", value.x1);
  packer.value("y1", value.y1);
  packer.value("x2", value.x2);
  packer.value("y2", value.y2);
  packer.object_end();
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
  DATAGUI_LOG_INIT();
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.begin()) {
      if (gui.series_begin()) {
        auto var = gui.edit_variable<Foo>();
        if (var.modified()) {
          std::cout << datapack::debug(*var) << std::endl;
        }
        gui.series_end();
      }
      gui.end();
    }
    gui.poll();
  }
  return 0;
}

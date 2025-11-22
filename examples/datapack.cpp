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

struct Person {
  std::string name;
  int age;
};

struct Foo {
  double number;
  bool test;
  Person person;
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

DATAPACK_INLINE(Person, value, packer) {
  packer.object_begin();
  packer.value("name", value.name);
  packer.value("age", value.age);
  packer.object_end();
}

DATAPACK_INLINE(Foo, value, packer) {
  packer.object_begin();
  packer.value("number", value.number);
  packer.value("test", value.test);
  packer.value("person", value.person);
#if 0
  packer.value("shape", value.shape);
  packer.value("points", value.points);
  packer.value("names", value.names);
#endif
  packer.object_end();
}
} // namespace datapack

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.begin()) {
      if (auto value = gui.edit<Foo>()) {
        std::cout << datapack::debug(*value) << std::endl;
      }
    }
    gui.poll();
  }
  return 0;
}

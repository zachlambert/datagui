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
  datagui::Color color = datagui::Color::Black();
  double scale;
};

namespace dpack {

DPACK_LABELLED_VARIANT(Shape, 2);
DPACK_LABELLED_VARIANT_DEF(Shape) = {"point", "line"};

DPACK_INLINE(Point, x, y)
DPACK_INLINE(Line, x1, y1, x2, y2)
DPACK_INLINE(Person, name, age)

inline void read(Reader& reader, Foo& foo) {
  reader.object_begin();
  reader.value("number", foo.number);
  reader.value("test", foo.test);
  reader.value("person", foo.person);
  reader.value("shape", foo.shape);
  reader.value("points", foo.points);
  reader.value("names", foo.names);
  reader.value("color", foo.color);
  reader.hint(HintRange(-1, 1));
  reader.value("scale", foo.scale);
  reader.object_end();
}
inline void write(Writer& writer, const Foo& foo) {
  writer.object_begin();
  writer.value("number", foo.number);
  writer.value("test", foo.test);
  writer.value("person", foo.person);
  writer.value("shape", foo.shape);
  writer.value("points", foo.points);
  writer.value("names", foo.names);
  writer.value("color", foo.color);
  writer.hint(HintRange(-1, 1));
  writer.value("scale", foo.scale);
  writer.object_end();
}

} // namespace dpack

int main() {
  datagui::Gui gui;

  while (gui.poll()) {
    gui.args().width_expand();
    gui.group();
    DATAGUI_SCOPE(gui);

    auto& value = gui.variable<Foo>();

    gui.args().text_size(20).text_color(datagui::Color::Blue());
    gui.text_box("Edit Foo");
    if (gui.edit_v("foo", value)) {
      std::cout << dpack::debug(value) << std::endl;
    }
    if (gui.button("Reset foo")) {
      value = Foo();
    }

    gui.args().text_size(20).text_color(datagui::Color::Blue());
    gui.text_box("Edit Foo List");
    if (auto value = gui.edit<std::vector<Foo>>("foo list")) {
      std::cout << dpack::debug(*value) << std::endl;
    }
  }
  return 0;
}

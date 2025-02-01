#include <datagui/window.hpp>
#include <datapack/common.hpp>
#include <datapack/util/debug.hpp>
#include <iostream>

struct Foo {
  int x;
  int y;
};

namespace datapack {
DATAPACK_INLINE(Foo, value, packer) {
  packer.object_begin();
  packer.value("x", value.x);
  packer.value("y", value.y);
  packer.object_end();
}
} // namespace datapack

int main() {
  datagui::Window::Config config;
  config.title = "Datagui Example 2";
  datagui::Style style;
  style.element.border_width = 1;

  Foo foo;

  datagui::Window window(config, style);

  while (window.running()) {
    window.render_begin();
    if (window.value(foo)) {
      std::cout << "Foo changed:\n" << datapack::debug(foo) << std::endl;
    }
    window.render_end();
  }
  return 0;
}

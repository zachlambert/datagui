#include <datagui/gui.hpp>
#include <vector>

int main() {
  using namespace datagui::literals;

  datagui::Gui gui;

  auto boxed_layout = [](datagui::VerticalLayoutStyle& style) {
    style.border_width = 2;
    style.padding = 20;
  };

  while (gui.running()) {
    gui.begin();
    if (gui.vertical_layout(boxed_layout)) {
      auto items = gui.variable<std::vector<std::string>>();
      if (gui.horizontal_layout()) {
        auto item = gui.variable<std::string>();
        gui.text_box("Item");
        gui.text_input(item);
        if (gui.button("Add")) {
          items.mut().push_back(item.immut());
        }
        gui.layout_end();
      }
      if (gui.vertical_layout(boxed_layout)) {
        std::size_t to_remove = items->size();
        for (std::size_t i = 0; i < items->size(); i++) {
          if (gui.horizontal_layout()) {
            if (auto value = gui.text_input((*items)[i])) {
              items.mut()[i] = *value;
            }
            if (gui.button("Remove")) {
              to_remove = i;
            }
            gui.layout_end();
          }
        }
        if (to_remove != items->size()) {
          items.mut().erase(items->begin() + to_remove);
        }
        gui.layout_end();
      }
      if (gui.vertical_layout(boxed_layout)) {
        auto text = gui.variable<std::string>("Foo");
        if (gui.horizontal_layout()) {
          auto text_overwrite = gui.variable<std::string>("");
          if (auto value = gui.text_input()) {
            text_overwrite.mut() = *value;
          }
          if (gui.button("Overwrite")) {
            text.mut() = text_overwrite.immut();
          }
          gui.layout_end();
        }
        if (auto value = gui.text_input([&]() { return *text; })) {
          text.mut() = *value;
        }
        gui.text_box([&]() { return *text; });
        gui.layout_end();
      }
      if (gui.vertical_layout(boxed_layout)) {
        auto foo = gui.variable<std::string>("Foo");
        auto bar = gui.variable<std::string>("Bar");
        gui.text_input(foo);
        gui.text_input(bar);
#if 0
        gui.text_box([&]() { return "Foo: " + *foo; });
        gui.text_box([&]() { return "Bar: " + *bar; });
#else
        gui.text_box("Foo: " + *foo);
        gui.text_box("Bar: " + *bar);
#endif
        gui.layout_end();
      }
      gui.layout_end();
    }
    gui.end();
  }
  return 0;
}

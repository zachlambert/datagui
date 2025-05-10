#include <datagui/gui.hpp>
#include <vector>

int main() {
  datagui::Gui gui;

  datagui::Style boxed_layout;
  boxed_layout.border_width(2);
  boxed_layout.padding(20);

  datagui::Style items_layout;
  items_layout.border_width(2);
  items_layout.padding(20);
  items_layout.layout_direction(datagui::Direction::Vertical);
  items_layout.height_fixed(100);
  items_layout.width_dynamic();

  while (gui.running()) {
    gui.begin();
    if (gui.vertical_layout(boxed_layout)) {
      auto items = gui.variable<std::vector<std::string>>();
      if (gui.horizontal_layout()) {
        auto item = gui.variable<std::string>();
        gui.text_box("Item");
        gui.text_input(item);
        if (gui.button("Add")) {
          std::vector<std::string> new_items = *items;
          new_items.push_back(*item);
          items.set(new_items);
        }
        gui.layout_end();
      }
      if (gui.vertical_layout(items_layout)) {
        std::size_t to_remove = items->size();
        for (std::size_t i = 0; i < items->size(); i++) {
          if (gui.horizontal_layout()) {
            if (auto value = gui.text_input((*items)[i])) {
              auto new_items = *items;
              new_items[i] = *value;
              items.set(new_items);
            }
            if (gui.button("Remove")) {
              to_remove = i;
            }
            gui.layout_end();
          }
        }
        if (to_remove != items->size()) {
          auto new_items = *items;
          new_items.erase(new_items.begin() + to_remove);
          items.set(new_items);
        }
        gui.layout_end();
      }
      if (gui.vertical_layout(boxed_layout)) {
        auto text = gui.variable<std::string>("Foo");
        if (gui.horizontal_layout()) {
          auto text_overwrite = gui.variable<std::string>();
          gui.text_input(text_overwrite);
          if (gui.button("Overwrite")) {
            text.set(*text_overwrite);
          }
          gui.layout_end();
        }
        gui.text_input(text);
        gui.text_box(*text);
        gui.layout_end();
      }
      if (gui.vertical_layout(boxed_layout)) {
        auto foo = gui.variable<std::string>("Foo");
        auto bar = gui.variable<std::string>("Bar");
        gui.text_input(foo);
        gui.text_input(bar);
        gui.text_box("Foo: " + *foo);
        gui.text_box("Bar: " + *bar);
        gui.layout_end();
      }
      gui.layout_end();
    }
    gui.end();
  }
  return 0;
}

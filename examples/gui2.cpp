#include <datagui/gui.hpp>
#include <vector>

int main() {
  datagui::Gui gui;

  datagui::Style boxed_layout;
  // boxed_layout.border_width(2);
  boxed_layout.series_outer_padding(20);

  datagui::Style items_layout;
  // items_layout.border_width(2);
  items_layout.series_outer_padding(20);
  items_layout.series_vertical();
  items_layout.series_length_fixed(100);
  items_layout.series_width_dynamic();

  while (gui.running()) {
    gui.begin();
    if (gui.series_begin(boxed_layout)) {
      auto items = gui.variable<std::vector<std::string>>();
      if (gui.series_begin(datagui::Style().series_horizontal())) {
        auto item = gui.variable<std::string>();
        gui.text_box("Item");
        gui.text_input(item);
        if (gui.button("Add")) {
          std::vector<std::string> new_items = *items;
          new_items.push_back(*item);
          items.set(new_items);
        }
        gui.series_end();
      }
      if (gui.series_begin(items_layout)) {
        std::size_t to_remove = items->size();
        for (std::size_t i = 0; i < items->size(); i++) {
          if (gui.series_begin()) {
            if (auto value = gui.text_input((*items)[i])) {
              auto new_items = *items;
              new_items[i] = *value;
              items.set(new_items);
            }
            if (gui.button("Remove")) {
              to_remove = i;
            }
            gui.series_end();
          }
        }
        if (to_remove != items->size()) {
          auto new_items = *items;
          new_items.erase(new_items.begin() + to_remove);
          items.set(new_items);
        }
        gui.series_end();
      }
      if (gui.series_begin(boxed_layout)) {
        auto text = gui.variable<std::string>("Foo");
        if (gui.series_begin()) {
          auto text_overwrite = gui.variable<std::string>();
          gui.text_input(text_overwrite);
          if (gui.button("Overwrite")) {
            text.set(*text_overwrite);
          }
          gui.series_end();
        }
        gui.text_input(text);
        gui.text_box(*text);
        gui.series_end();
      }
      if (gui.series_begin(boxed_layout)) {
        auto foo = gui.variable<std::string>("Foo");
        auto bar = gui.variable<std::string>("Bar");
        gui.text_input(foo);
        gui.text_input(bar);
        gui.text_box("Foo: " + *foo);
        gui.text_box("Bar: " + *bar);
        gui.series_end();
      }
      gui.series_end();
    }
    gui.end();
  }
  return 0;
}

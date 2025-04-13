#include <datagui/gui.hpp>
#include <vector>

int main() {
  using namespace datagui::literals;

  datagui::Gui gui;

  while (gui.running()) {
    gui.begin();
    if (gui.vertical_layout()) {
      auto items = gui.variable<std::vector<std::string>>();
      if (gui.horizontal_layout()) {
        auto item = gui.variable<std::string>();
        gui.text_box("Item");
        gui.text_input(item);
        if (gui.button("Add")) {
          items.mut().push_back(*item);
        }
        gui.layout_end();
      }
      if (gui.vertical_layout()) {
        items.depend_immutable();
        auto to_remove = items->end();
        for (auto iter = items->begin(); iter != items->end(); iter++) {
          if (gui.horizontal_layout()) {
            gui.text_box(*iter);
            if (gui.button("Remove")) {
              to_remove = iter;
            }
            gui.layout_end();
          }
        }
        if (to_remove != items->end()) {
          items.mut().erase(to_remove);
        }
        gui.layout_end();
      }
      gui.layout_end();
    }
    gui.end();
  }
  return 0;
}

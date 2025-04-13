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
      gui.layout_end();
    }
    gui.end();
  }
  return 0;
}

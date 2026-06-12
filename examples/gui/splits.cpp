#include <datagui/gui.hpp>

int main() {
  dgui::Gui gui;
  gui.open();
  using dgui::Color;

  while (gui.poll()) {
    gui.vsplit(0.3);
    DGUI_SCOPE(gui);

    gui.group();
    {
      DGUI_SCOPE(gui);

      auto& counter = gui.variable<int>(0);
      gui.tabs(1);
      {
        DGUI_SCOPE(gui);
        if (gui.tab_group("Display")) {
          DGUI_SCOPE(gui);
          gui.text_box(std::to_string(counter));
        }
        if (gui.tab_group("Increment")) {
          DGUI_SCOPE(gui);
          if (gui.button("Increment")) {
            counter++;
          };
        }
        if (gui.tab_group("Decrement")) {
          DGUI_SCOPE(gui);
          if (gui.button("Decrement")) {
            counter--;
          };
        }
      }

      gui.args().grid(-1, 3);
      gui.group();
      {
        DGUI_SCOPE(gui);
        gui.text_box("aaa");
        gui.text_box("bbb");
        gui.text_box("ccc");
        gui.args().num_cells(2);
        gui.text_box("ddddddddddddddd");
        gui.text_box("ee");
        gui.args().num_cells(3).text_input_wrap();
        std::ignore = gui.text_input("fffffffffffff");
      }
    }

    gui.args().width_expand();
    gui.hsplit(0.5);
    {
      DGUI_SCOPE(gui);

      auto& popup_open = gui.variable<bool>(false);
      gui.args().retain();
      if (gui.popup(popup_open, "Popup", 200, 200)) {
        DGUI_SCOPE(gui);
        auto& counter = gui.variable<int>(0);
        gui.text_box("Popup " + std::to_string(counter));
        if (gui.button("Increment")) {
          counter++;
        }
      }

      gui.args().tight();
      if (gui.dropdown("Third")) {
        DGUI_SCOPE(gui);
        gui.text_box("One");

        gui.args().dropdown_horizontal().tight();
        if (gui.dropdown("Two - Click!")) {
          DGUI_SCOPE(gui);
          gui.text_box("Item 0");
          gui.text_box("Item 1");
          if (gui.button("Spawn popup")) {
            popup_open = true;
          }
          gui.text_box("Item 3");
        }
        gui.text_box("Three");
        gui.text_box("Four");
      }

      gui.args().split_fixed();
      gui.vsplit(0.6);
      {
        DGUI_SCOPE(gui);

        gui.args().width_expand();
        gui.group();
        {
          DGUI_SCOPE(gui);
          gui.text_box("Fourth");
          for (std::size_t i = 0; i < 20; i++) {
            gui.text_box("Fourth [" + std::to_string(i) + "]");
          }
        }

        gui.text_box("Fifth");
      }
    }
  }
  return 0;
}

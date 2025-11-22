#include <datagui/element/key_list.hpp>"
#include <datagui/gui.hpp>

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.begin()) {
      if (gui.series_begin()) {

        if (gui.series_begin()) {
          struct Person {
            const std::string name;
            std::string desc;
          };
          auto persons = gui.variable<std::vector<Person>>();

          if (gui.series_begin()) {
            for (std::size_t i = 0; i < persons->size(); i++) {
              const auto& person = (*persons)[i];
              gui.key(person.name);
              if (!gui.series_begin()) {
                continue;
              }
              gui.text_box(person.name);
              if (gui.labelled_begin("Desc.")) {
                if (auto value = gui.text_input(person.desc)) {
                  persons.mut()[i].desc = *value;
                }
                gui.labelled_end();
              }
              gui.series_end();
            }
            gui.series_end();
          }
          if (gui.series_begin()) {
            auto new_name = gui.variable<std::string>();
            auto new_desc = gui.variable<std::string>();
            if (gui.labelled_begin("Name")) {
              gui.text_input(new_name);
              gui.labelled_end();
            }
            if (gui.labelled_begin("Desc")) {
              gui.text_input(new_desc);
              gui.labelled_end();
            }
            if (gui.button("Add")) {
              persons.mut().push_back({*new_name, *new_desc});
            }
            gui.series_end();
          }
          gui.series_end();
        }

        if (gui.series_begin()) {
          auto keys = gui.variable<datagui::KeyList>();

          if (gui.series_begin()) {
            gui.series_end();
          }
        }

        gui.series_end();
      }
      gui.end();
    }
    gui.poll();
  }
  return 0;
}

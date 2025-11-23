#include <datagui/element/key_list.hpp>
#include <datagui/gui.hpp>

void edit_list_1(datagui::Gui& gui) {
  if (!gui.series()) {
    return;
  }

  struct Person {
    const std::string name;
    std::string desc;
  };
  auto persons = gui.variable<std::vector<Person>>();

  if (gui.series()) {
    for (std::size_t i = 0; i < persons->size(); i++) {
      const auto& person = (*persons)[i];
      gui.key(person.name);
      if (!gui.series()) {
        continue;
      }
      gui.text_box(person.name);
      if (gui.labelled("Desc.")) {
        gui.text_input(person.desc, [=](const std::string& value) {
          persons.mut()[i].desc = value;
        });
        gui.end();
      }
      gui.end();
    }
    gui.end();
  }
  if (gui.series()) {
    auto new_name = gui.variable<std::string>();
    gui.text_input(new_name);

    auto new_desc = gui.variable<std::string>();
    gui.text_input(new_desc);

    gui.button("Add", [=]() {
      persons.mut().push_back({*new_name, *new_desc});
    });
    gui.end();
  }
  gui.end();
}

void edit_list_2(datagui::Gui& gui) {
  if (gui.series()) {
    auto keys = gui.variable<datagui::KeyList>();
    if (gui.series()) {
      gui.end();
    }
  }
}

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.series()) {
      edit_list_1(gui);
      edit_list_2(gui);
      gui.end();
    }
    gui.poll();
  }
  return 0;
}

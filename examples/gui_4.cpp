#include <datagui/element/key_list.hpp>
#include <datagui/gui.hpp>

void edit_list_1(datagui::Gui& gui) {
  if (!gui.series()) {
    return;
  }

  struct Person {
    std::string name;
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
      gui.button("Remove", [=]() {
        persons.mut().erase(persons->begin() + i);
      });
      gui.end();
    }
    gui.end();
  }
  if (gui.series()) {
    auto new_name = gui.variable<std::string>();
    gui.text_input(new_name);

    auto new_desc = gui.variable<std::string>();
    gui.text_input(new_desc);

    auto error = gui.variable<std::string>();

    gui.button("Add", [=]() {
      if (new_name->empty()) {
        error.set("Cannot have an empty name");
        return;
      }
      auto existing = std::find_if(
          persons->begin(),
          persons->end(),
          [&](const Person& person) { return person.name == *new_name; });
      if (existing != persons->end()) {
        error.set("Person with name '" + *new_name + "' already exists");
        return;
      }
      persons.mut().push_back({*new_name, *new_desc});
      error.mut().clear();
    });

    gui.key<std::string>("error box");
    if (!error->empty()) {
      gui.text_box("Error: " + *error);
    }

    gui.end();
  }
  gui.end();
}

void edit_list_2(datagui::Gui& gui) {
  if (!gui.series()) {
    return;
  }

  auto keys = gui.variable<datagui::KeyList>();
  if (gui.series()) {
    for (std::size_t i = 0; i < keys->size(); i++) {
      gui.args().horizontal();
      gui.key((*keys)[i]);
      if (gui.series()) {
        gui.text_input("", {});
        gui.button("Remove", [=]() { keys.mut().remove(i); });
        gui.end();
      }
    }
    gui.end();
  }
  if (gui.series()) {
    gui.button("Push", [=]() { keys.mut().append(); });
    gui.end();
  }
  gui.end();
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

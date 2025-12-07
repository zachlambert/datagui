#include <algorithm>
#include <datagui/element/key_list.hpp>
#include <datagui/gui.hpp>

void edit_list_1(datagui::Gui& gui) {
  if (!gui.group()) {
    return;
  }

  struct Person {
    std::string name;
    std::string desc;
  };
  auto persons = gui.variable<std::vector<Person>>();

  gui.args().border().width_expand();
  if (gui.group()) {
    for (std::size_t i = 0; i < persons->size(); i++) {
      const auto& person = (*persons)[i];
      gui.key(person.name);
      if (!gui.group()) {
        continue;
      }

      gui.text_box(person.name);

      gui.args().horizontal();
      if (gui.group()) {
        gui.text_box("Desc.");
        gui.text_input(person.desc, [=](const std::string& value) {
          persons.mut()[i].desc = value;
        });
        gui.end();
      }

      std::string name = person.name;
      gui.button("Remove", [=]() {
        auto iter = std::find_if(
            persons->begin(),
            persons->end(),
            [name](const auto& person) { return person.name == name; });
        assert(iter != persons->end());
        persons.mut().erase(iter);
      });
      gui.end();
    }
    gui.end();
  }

  gui.args().border();
  if (gui.group()) {
    auto new_name = gui.variable<std::string>();
    auto new_desc = gui.variable<std::string>();

    gui.args().grid(-1, 2);
    if (gui.group()) {
      gui.text_box("Name");
      gui.text_input(new_name);
      gui.text_box("Description");
      gui.text_input(new_desc);
      gui.end();
    }

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

    if (!error->empty()) {
      gui.key<std::string>("error box");
      gui.text_box("Error: " + *error);
    }

    gui.end();
  }
  gui.end();
}

void edit_list_2(datagui::Gui& gui) {
  if (!gui.group()) {
    return;
  }

  auto keys = gui.variable<datagui::KeyList>();
  if (gui.group()) {
    for (std::size_t i = 0; i < keys->size(); i++) {
      gui.args().horizontal();
      auto key = (*keys)[i];
      gui.key(key);
      if (gui.group()) {
        gui.text_input("", {});
        gui.button("Remove", [=]() { keys.mut().remove(key); });
        gui.end();
      }
    }
    gui.end();
  }
  if (gui.group()) {
    gui.button("Push", [=]() { keys.mut().append(); });
    gui.end();
  }
  gui.end();
}

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.group()) {
      edit_list_1(gui);
      edit_list_2(gui);
      gui.end();
    }
    gui.poll();
  }
  return 0;
}

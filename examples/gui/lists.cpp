#include <algorithm>
#include <datagui/element/key_list.hpp>
#include <datagui/gui.hpp>

void edit_list_1(dgui::Gui& gui) {
  gui.group();
  DGUI_SCOPE(gui);

  struct Person {
    std::string name;
    std::string desc;
  };
  auto& persons = gui.variable<std::vector<Person>>();

  gui.args().border().width_expand();
  gui.group();
  {
    DGUI_SCOPE(gui);
    size_t i = 0;
    while (i < persons.size()) {
      gui.key(persons[i].name);
      gui.group();
      DGUI_SCOPE(gui);

      gui.text_box(persons[i].name);

      gui.args().horizontal();
      gui.group();
      {
        DGUI_SCOPE(gui);
        gui.text_box("Desc.");
        gui.text_input_v(persons[i].desc);
      }

      if (gui.button("Remove")) {
        persons.erase(persons.begin() + i);
        continue;
      }
      i++;
    }
  }

  gui.args().border();
  gui.group();
  {
    DGUI_SCOPE(gui);
    auto& new_name = gui.variable<std::string>();
    auto& new_desc = gui.variable<std::string>();

    gui.args().grid(-1, 2);
    gui.group();
    {
      DGUI_SCOPE(gui);
      gui.text_box("Name");
      gui.text_input_v(new_name);
      gui.text_box("Description");
      gui.text_input_v(new_desc);
    }

    auto& error = gui.variable<std::string>();

    if (gui.button("Add")) {
      if (new_name.empty()) {
        error = "Cannot have an empty name";
      } else {
        auto existing = std::find_if(
            persons.begin(),
            persons.end(),
            [&](const Person& person) {
              return person.name == new_name;
            });
        if (existing != persons.end()) {
          error = "Person with name '" + new_name + "' already exists";
        } else {
          persons.push_back({new_name, new_desc});
          error.clear();
        }
      }
    }

    if (!error.empty()) {
      gui.key<std::string>("error box");
      gui.text_box("Error: " + error);
    }
  }
}

void edit_list_2(dgui::Gui& gui) {
  gui.args().border().width_expand();
  gui.group();
  DGUI_SCOPE(gui);

  auto& keys = gui.variable<dgui::KeyList>();
  gui.group();
  {
    DGUI_SCOPE(gui);
    size_t i = 0;
    while (i < keys.size()) {
      gui.args().horizontal();
      gui.key(keys[i]);
      gui.group();
      {
        DGUI_SCOPE(gui);
        std::ignore = gui.text_input("");

        const bool remove = gui.button("Remove");
        const bool insert = gui.button("Insert");
        if (remove) {
          keys.remove(keys[i]);
          // No change to i
        } else if (insert) {
          keys.insert(i + 1);
          i++;
        } else {
          i++;
        }
      }
    }
    if (keys.size() == 0) {
      if (gui.button("Insert")) {
        keys.append();
      };
    }
  }
}

int main() {
  dgui::Gui gui;
  gui.open();

  while (gui.poll()) {
    gui.group();
    DGUI_SCOPE(gui);

    edit_list_1(gui);
    edit_list_2(gui);
  }
  return 0;
}

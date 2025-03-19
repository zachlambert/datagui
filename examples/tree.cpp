#include <datagui/internal/tree.hpp>
#include <datagui/internal/vector_map.hpp>
#include <iostream>

int main() {
  datagui::Tree tree([]() { return 0; }, [](int) {});

  std::string name_value;
  int name_node;

  while (true) {
    tree.begin();
    tree.next();
    if (tree.down()) {
      tree.next();
      name_node = tree.current();
      auto name = tree.data(&name_value);
      tree.next();
      if (tree.down()) {
        tree.next();
        if (name) {
          std::cout << "Name: " << *name << std::endl;
        }
        tree.up();
      }
      tree.up();
    }
    tree.end();

    {
      std::string line;
      std::cout << "> ";
      std::getline(std::cin, line);
      if (!line.empty()) {
        name_value = line;
        tree.set_modified(name_node);
      }
    }
  }
}

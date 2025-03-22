#include <datagui/tree/tree.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Tree tree([](const State&) {});

  std::string name_value;
  int name_node;

  while (true) {
    tree.begin();
    tree.container_next([](State&) {});
    if (tree.container_down()) {
      tree.container_next([](State&) {});
      name_node = tree.current();
      auto name = tree.data(&name_value);
      tree.container_next([](State&) {});
      if (tree.container_down()) {
        tree.container_next([](State&) {});
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

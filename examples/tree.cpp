#include <datagui/tree/tree.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Tree tree([](const State&) {});

  while (true) {
    tree.begin();
    tree.container_next([](State&) {});

    if (tree.container_down()) {
      auto name = tree.data_parent<std::string>();
      tree.container_next([](State&) {});
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
        tree.root().data<std::string>().mut() = line;
      }
    }
  }
}

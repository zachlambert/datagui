#include <datagui/tree/tree.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Tree tree([](Tree::ConstPtr) {});

  while (true) {
    auto name = tree.variable<std::string>();

    tree.begin();
    tree.container_next([](State&) {});

    if (tree.container_down()) {
      tree.container_next([](State&) {});
      tree.container_next([](State&) {});
      if (tree.container_down()) {
        tree.container_next([](State&) {});
        if (name.modified()) {
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
        name.mut() = line;
      }
    }
  }
}

#include <datagui/tree/tree.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Tree tree;
  Variable<std::string> name;

  while (true) {
    tree.begin();
    tree.next();

    if (tree.down_if()) {
      name = tree.variable<std::string>();

      tree.next();
      tree.next();
      if (tree.down_if()) {
        tree.next();
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
        name.set(line);
      }
    }
  }
}

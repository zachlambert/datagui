#include <datagui/tree/tree.hpp>
#include <iostream>

int main() {
  DATAGUI_LOG_INIT();

  using namespace datagui;

  Tree tree;
  Var<std::string> name;

  while (true) {
    if (tree.begin()) {
      tree.next();

      if (tree.down_if()) {
        name = tree.variable<std::string>();

        tree.next();
        tree.next();
        if (tree.down_if()) {
          tree.on_variable(ConstVar<std::string>(name));
          tree.next();
          std::cout << "Name: " << *name << std::endl;
          tree.up();
        }
        tree.up();
      }
      tree.end();
    }

    {
      std::string line;
      std::cout << "> ";
      std::getline(std::cin, line);
      if (!line.empty()) {
        *name = line;
        name.set_modified();
      }
    }
  }
}

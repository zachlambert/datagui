#include <datagui/tree/tree.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Tree tree;
  Variable<std::string> name;

  while (true) {
    name = tree.variable<std::string>();

    tree.begin();
    tree.next(ElementType::HorizontalLayout);

    if (tree.down_if()) {
      tree.next(ElementType::TextInput);
      tree.next(ElementType::TextBox);
      if (tree.down_if()) {
        tree.next(ElementType::TextBox);
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

#include <datagui/tree/tree.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Tree tree;

  auto node = tree.root();
  if (!node.exists()) {
    node.create(PropsType::Button);
  }
}

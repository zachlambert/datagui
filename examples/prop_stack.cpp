#include <datagui/prop_stack.hpp>
#include <iostream>

int main() {
  datagui::PropStack props;
  props.push<int>(0, 10);
  props.push<std::string>(1, "hello");

  for (datagui::Prop prop : props) {
    if (auto value = prop.as<int>()) {
      std::cout << *value << std::endl;
    } else if (auto value = prop.as<std::string>()) {
      std::cout << *value << std::endl;
    }
  }

  return 0;
}

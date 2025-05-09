#include <datagui/prop_stack.hpp>
#include <iostream>

int main() {
  enum class PropType { Count, Name };
  auto type_name = [](PropType type) -> const char* {
    switch (type) {
    case PropType::Count:
      return "Count";
      break;
    case PropType::Name:
      return "Name";
      break;
    }
    return "unknown";
  };

  datagui::PropStack<PropType> props;
  props.push<int>(PropType::Count, 10);
  props.push<std::string>(PropType::Name, "hello");

  std::cout << "Count: " << *props.get<int>(PropType::Count) << std::endl;
  std::cout << "Name:  " << *props.get<std::string>(PropType::Name)
            << std::endl;

  std::cout << "All values" << std::endl;
  for (datagui::Prop prop : props) {
    std::cout << "- Type:  " << type_name(prop.id()) << std::endl;
    std::cout << "  Value: ";
    if (auto value = prop.as<int>()) {
      std::cout << *value << std::endl;
    } else if (auto value = prop.as<std::string>()) {
      std::cout << *value << std::endl;
    } else {
      std::cout << "Unknown" << std::endl;
    }
  }

  return 0;
}

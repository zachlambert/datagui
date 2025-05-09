#include <datagui/types/prop_stack.hpp>
#include <iostream>

enum class Prop { Count, Name };
const char* prop_name(const Prop& prop) {
  switch (prop) {
  case Prop::Count:
    return "Count";
    break;
  case Prop::Name:
    return "Name";
    break;
  }
  return "unknown";
};

int main() {

  datagui::PropSet<Prop> props;
  props.insert<int>(Prop::Count, 10);
  props.insert<std::string>(Prop::Name, "hello");
  props.insert<int>(Prop::Count, 20);

  std::cout << "Count: " << *props.get<int>(Prop::Count) << std::endl;
  std::cout << "Name:  " << *props.get<std::string>(Prop::Name) << std::endl;

  std::cout << "All values" << std::endl;
  for (auto prop : props) {
    std::cout << "- Type:  " << prop_name(prop.key()) << std::endl;
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

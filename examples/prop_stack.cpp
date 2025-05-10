#include <datagui/types/prop_stack.hpp>
#include <iostream>

enum class Prop { Count, Text };
const char* prop_name(const Prop& prop) {
  switch (prop) {
  case Prop::Count:
    return "Count";
    break;
  case Prop::Text:
    return "Text";
    break;
  }
  return "unknown";
};

// Works with: PropContainer<K>, PropSet<K>, PropStack<K>
template <typename Container>
void print_props(const Container& props) {
  std::cout << "Count: ";
  if (auto value = props.template get<int>(Prop::Count)) {
    std::cout << *props.template get<int>(Prop::Count);
  } else {
    std::cout << "<none>";
  }
  std::cout << std::endl;

  std::cout << "Text: ";
  if (auto value = props.template get<std::string>(Prop::Text)) {
    std::cout << *props.template get<std::string>(Prop::Text);
  } else {
    std::cout << "<none>";
  }
  std::cout << std::endl;

  std::cout << "All values" << std::endl;
  for (auto prop : props) {
    std::cout << "- Type:  " << prop_name(prop.key()) << std::endl;
    std::cout << "  Value: ";
    if (auto value = prop.template as<int>()) {
      std::cout << *value << std::endl;
    } else if (auto value = prop.template as<std::string>()) {
      std::cout << *value << std::endl;
    } else {
      std::cout << "<unknown>" << std::endl;
    }
  }
}

int main() {

  datagui::PropSet<Prop> props;
  props.insert<std::string>(Prop::Text, "adsf");
  props.insert<int>(Prop::Count, 10);
  props.insert<std::string>(Prop::Text, "hello");
  props.insert<int>(Prop::Count, 20);

  std::cout << "========" << std::endl;
  print_props(props);

#if 0
  datagui::PropStack<Prop> stack;
  stack.push<std::string>(Prop::Text, "foo");

  std::cout << "========" << std::endl;
  print_props(stack);

  std::cout << "========" << std::endl;
  // stack.push_checkpoint();
  stack.push(props);
  print_props(stack);

  std::cout << "========" << std::endl;
#if 0
  stack.pop_checkpoint();
  stack.push(props);
#endif
#endif

  return 0;
}

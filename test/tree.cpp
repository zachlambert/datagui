#include "datagui/tree/tree.hpp"
#include <gtest/gtest.h>

TEST(Tree, CreateElements) {
  using namespace datagui;

  Tree tree;

  auto node = tree.root();
  ASSERT_FALSE(node.exists());
  node.create(PropsType::Series);
  ASSERT_TRUE(node.exists());
  ASSERT_EQ(node.props_type(), PropsType::Series);
  {
    auto& element = *node;
    element.position = Vecf(1, 2);
    auto& props = node.series_props();
    props.bg_color = Color::Red();
  }

  node = node.child();
  ASSERT_FALSE(node.exists());
  node.create(PropsType::Button);
  ASSERT_TRUE(node.exists());
  ASSERT_EQ(node.props_type(), PropsType::Button);
  {
    auto& element = *node;
    element.position = Vecf(1, 2);
    auto& props = node.button_props();
    props.down = true;
  }

  node = node.next();
  ASSERT_FALSE(node.exists());
  node.create(PropsType::TextInput);
  ASSERT_TRUE(node.exists());
  ASSERT_EQ(node.props_type(), PropsType::TextInput);
  {
    auto& element = *node;
    element.position = Vecf(1, 2);
    auto& props = node.text_input_props();
    props.text = "hello";
  }
}

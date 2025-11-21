#include "datagui/element/tree.hpp"
#include <gtest/gtest.h>

TEST(Tree, CreateElements) {
  using namespace datagui;

  Tree tree;

  auto node = tree.root();
  ASSERT_FALSE(node.exists());
  node.create(Type::Series);
  ASSERT_TRUE(node.exists());
  ASSERT_EQ(node.type(), Type::Series);
  {
    auto& state = node.state();
    state.position = Vecf(1, 2);
    auto& series = node.series();
    series.bg_color = Color::Red();
  }

  node = node.child();
  ASSERT_FALSE(node.exists());
  node.create(Type::Button);
  ASSERT_TRUE(node.exists());
  ASSERT_EQ(node.type(), Type::Button);
  {
    auto& state = node.state();
    state.position = Vecf(1, 2);
    auto& button = node.button();
    button.down = true;
  }

  node = node.next();
  ASSERT_FALSE(node.exists());
  node.create(Type::TextInput);
  ASSERT_TRUE(node.exists());
  ASSERT_EQ(node.type(), Type::TextInput);
  {
    auto& state = node.state();
    state.position = Vecf(1, 2);
    auto& text_input = node.text_input();
    text_input.text = "hello";
  }
}

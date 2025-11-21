#include "datagui/tree/tree.hpp"
#include <gtest/gtest.h>

TEST(Tree, CreateElements) {
  using namespace datagui;

  Tree tree;

  auto node = tree.root();
  ASSERT_FALSE(node.exists());
  node.create(PropsType::Button);
  ASSERT_TRUE(node.exists());
  ASSERT_EQ(node.props_type(), PropsType::Button);
}

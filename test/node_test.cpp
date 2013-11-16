#include <iostream>
#include <memory>

#include "gtest/gtest.h"
#include "map.h"

using ii_map = immutable::map<int, int>;
using namespace std;

TEST(trie_node, can_calculate_child_position)
{
  uint32_t testpop {(1 | (1 << 3) | (1 << 6))};
  ii_map::trie_node test_node(testpop, vector<shared_ptr<ii_map::node>>(3));

  ASSERT_EQ(0, test_node.child_order(0));
  ASSERT_EQ(1, test_node.child_order(3));
  ASSERT_EQ(2, test_node.child_order(6));
}

TEST(trie_node, can_check_child_presence)
{
  uint32_t testpop {(1 | (1 << 3) | (1 << 6))};
  ii_map::trie_node test_node(testpop, vector<shared_ptr<ii_map::node>>(3));

  ASSERT_TRUE(test_node.child_present(0));
  ASSERT_TRUE(test_node.child_present(3));
  ASSERT_TRUE(test_node.child_present(6));

  ASSERT_FALSE(test_node.child_present(1));
  ASSERT_FALSE(test_node.child_present(2));
  ASSERT_FALSE(test_node.child_present(4));
  ASSERT_FALSE(test_node.child_present(12));
  ASSERT_FALSE(test_node.child_present(31));
}

TEST(trie_node, can_set_value)
{
  ii_map::trie_node test_node;
  ii_map::value_type pair(1, 10);
  size_t hash = 1;

  ii_map::trie_node result_node = *static_cast<ii_map::trie_node *>(test_node.set(hash, pair).release());

  ASSERT_NE(&result_node, &test_node);
  ASSERT_FALSE(test_node.child_present(1));
  ASSERT_TRUE(result_node.child_present(1));

  ASSERT_EQ(0, test_node.child_order(1));

  shared_ptr<ii_map::value_node> val_node = static_pointer_cast<ii_map::value_node>(result_node.get_child(1));

  ASSERT_EQ(1, val_node->get(1).first);
  ASSERT_EQ(10, val_node->get(1).second);
}

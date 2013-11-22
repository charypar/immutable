#include <iostream>
#include <memory>

#include "gtest/gtest.h"
#include "map.h"

using namespace std;

struct hash_stub {
  size_t operator()(int key) {
    return (1 << 5) | 1;
  }
};

using ii_map = immutable::map<int, int>;
using ii_map_s = immutable::map<int, int, hash_stub>;

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
  ii_map::node_ptr test_node_ptr = make_shared<ii_map::trie_node>();
  ii_map::trie_node test_node = *static_cast<ii_map::trie_node *>(test_node_ptr.get());
  ii_map::value_type pair(1, 10);
  size_t hash = 1;

  ii_map::trie_node result_node = *static_cast<ii_map::trie_node *>(test_node_ptr->set(hash, 0, pair, test_node_ptr).release());

  ASSERT_NE(&result_node, &test_node);
  ASSERT_FALSE(test_node.child_present(1));
  ASSERT_TRUE(result_node.child_present(1));

  ASSERT_EQ(0, test_node.child_order(1));

  shared_ptr<ii_map::value_node> val_node = static_pointer_cast<ii_map::value_node>(result_node.get_child(1));

  ASSERT_EQ(1, val_node->get(1, 1).first);
  ASSERT_EQ(10, val_node->get(1, 1).second);
}

TEST(trie_node, can_set_two_values_with_same_top_level_hash)
{
  ii_map_s::node_ptr test_ptr = make_shared<ii_map_s::trie_node>();

  ii_map_s::value_type v1(1, 10);
  ii_map_s::value_type v2(2, 20);

  // same 5 LSB
  size_t h1 = (1 << 5) | 1;
  size_t h2 = (1 << 6) | 1;

  ii_map_s::node_ptr test2 = test_ptr->set(h1, 0, v1, test_ptr);
  unique_ptr<ii_map_s::node> root_ptr = test2->set(h2, 0, v2, test2);
  ii_map_s::trie_node root = *static_cast<ii_map_s::trie_node *>(root_ptr.get());

  // check that a child trie_node was created

  ASSERT_TRUE(root.child_present(1));
  ASSERT_FALSE(root.child_present(2));
  ASSERT_FALSE(root.child_present(6));
  ASSERT_FALSE(root.child_present(7));

  ii_map_s::node_ptr child_ptr = root.get_child(1);
  ii_map_s::trie_node child = *static_cast<ii_map_s::trie_node *>(child_ptr.get());
  // and it has two child value_nodes

  ASSERT_TRUE(child.child_present(1));
  ASSERT_TRUE(child.child_present(2));
  ASSERT_FALSE(child.child_present(6));
  ASSERT_FALSE(child.child_present(7));

  ii_map_s::value_node v1n = *static_cast<ii_map_s::value_node *>(child.get_child(1).get());
  ii_map_s::value_node v2n = *static_cast<ii_map_s::value_node *>(child.get_child(2).get());

  // with the correct pairs
  ASSERT_EQ(1, v1n.get(1, 1).first);
  ASSERT_EQ(10, v1n.get(1, 1).second);

  ASSERT_EQ(2, v2n.get(2, 2).first);
  ASSERT_EQ(20, v2n.get(2, 2).second);
}

TEST(trie_node, can_erase_value_with_siblings)
{
  ii_map_s::node_ptr test_ptr = make_shared<ii_map_s::trie_node>();

  ii_map_s::value_type v1(1, 10);
  ii_map_s::value_type v2(2, 20);
  ii_map_s::value_type v3(3, 30);

  // hashes are chosen so that all three elements end up in a single 2nd level trie node
  size_t h1 = (1 << 5) | 1;
  size_t h2 = (2 << 5) | 1;
  size_t h3 = (3 << 5) | 1;

  ii_map_s::node_ptr test2 = test_ptr->set(h1, 0, v1, test_ptr);
  ii_map_s::node_ptr test3 = test2->set(h2, 0, v2, test_ptr);
  ii_map_s::node_ptr test4 = test3->set(h3, 0, v3, test2);

  // remove first element
  ii_map_s::node_ptr root_ptr = test4->erase(h1, 0, 1);

  ii_map_s::trie_node root = *static_cast<ii_map_s::trie_node *>(root_ptr.get());

  ASSERT_TRUE(root.child_present(1));

  ii_map_s::node_ptr child_ptr = root.get_child(1);
  ii_map_s::trie_node child = *static_cast<ii_map_s::trie_node *>(child_ptr.get());

  ASSERT_FALSE(child.child_present(1));
  ASSERT_TRUE(child.child_present(2));
  ASSERT_TRUE(child.child_present(3));
}



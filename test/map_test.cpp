#include <iostream>

#include "gtest/gtest.h"
#include "map.h"

using int_int_map = immutable::map<int, int>;

TEST(map, can_be_initialized)
{
  int_int_map test_map;
}

TEST(map, can_store_single_pair)
{
  int_int_map test_map;

  int_int_map new_map = test_map.set(1, 10);

  ASSERT_NE(&test_map, &new_map);
  ASSERT_EQ(10, new_map.at(1));
}

TEST(map, can_store_multiple_pairs)
{
  int_int_map test_map;

  int_int_map new_map = test_map.set(1, 10);
  int_int_map another_map = new_map.set(2, 20);

  ASSERT_NE(&test_map, &new_map);
  ASSERT_NE(&test_map, &another_map);
  ASSERT_NE(&new_map, &another_map);

  ASSERT_EQ(10, new_map.at(1));

  ASSERT_EQ(10, another_map.at(1));
  ASSERT_EQ(20, another_map.at(2));
}

TEST(map, can_remove_a_pair)
{
  int_int_map test_map;

  int_int_map new_map = test_map.set(1, 10).set(2, 20);
  int_int_map another_map = new_map.erase(1);

  ASSERT_NE(&test_map, &new_map);
  ASSERT_NE(&test_map, &another_map);
  ASSERT_NE(&new_map, &another_map);

  ASSERT_EQ(10, new_map.at(1));
  ASSERT_EQ(20, new_map.at(2));

  ASSERT_EQ(20, another_map.at(2));
  ASSERT_THROW(another_map.at(1), std::out_of_range);
}

TEST(map, can_handle_a_series_of_operations)
{
  int_int_map test_map;

  int_int_map result = test_map.set(1, 10).set(2, 20).erase(2).erase(1).set(3, 30).set(2, 20);

  ASSERT_EQ(30, result.at(3));
  ASSERT_EQ(20, result.at(2));

  ASSERT_THROW(result.at(1), std::out_of_range);
}

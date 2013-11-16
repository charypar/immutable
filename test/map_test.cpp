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
  // ASSERT_EQ(10, new_map[1]);
}

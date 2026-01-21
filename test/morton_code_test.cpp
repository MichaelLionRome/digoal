
extern "C" {
#include <block.h>
#include <block_frontier.h>
#include <block_topology.h>
#include <queries_state.h>
}

#include <algorithm>
#include <iostream>
#include <vector>

#include <gtest/gtest.h>

TEST(test_morton_code, test_convert_back_1) {
  struct morton_code mc;
  init_morton_code(&mc, 3);

  add_element_morton_code(&mc, 0, 1);
  add_element_morton_code(&mc, 1, 1);
  add_element_morton_code(&mc, 2, 1);

  ulong col_expected = 0;
  ulong row_expected = 7;

  struct pair2dl result;

  convert_morton_code_to_coordinates(&mc, &result);

  EXPECT_EQ(col_expected, result.col) << "COL not matching";
  EXPECT_EQ(row_expected, result.row) << "ROW not matching";

  clean_morton_code(&mc);
}
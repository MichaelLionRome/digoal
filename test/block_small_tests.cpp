//
// Created by cristobal on 22-09-21.
//

#include <gtest/gtest.h>

#include "block_wrapper.hpp"

extern "C" {
#include <block.h>
#include <block_frontier.h>
#include <block_topology.h>
#include <queries_state.h>
}

TEST(block_small_tests, test1) {
  BlockWrapper block_wrapper(64, 256);
  auto value = (1UL << 37UL);
  block_wrapper.insert(value, value);
  ASSERT_TRUE(block_wrapper.has(value, value));
}

TEST(block_small_tests, test2) {
  // tree_depth cant be higher than 63
  BlockWrapper block_wrapper(64, 256);
  unsigned long value = (1UL << 63UL);
  block_wrapper.insert(value, value);
  std::set<std::pair<unsigned long, unsigned long>> data;
  scan_points_interactively(
      block_wrapper.get_root(), block_wrapper.get_qs(),
      [](unsigned long col, unsigned long row, void *rs) {
        auto &data = *reinterpret_cast<
            std::set<std::pair<unsigned long, unsigned long>> *>(rs);
        data.insert({col, row});
      },
      &data);
  ASSERT_TRUE(data.find({value, value}) != data.end());
}

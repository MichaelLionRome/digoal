//
// Created by cristobal on 22-09-21.
//

#include <gtest/gtest.h>

extern "C" {
#include <block.h>
#include <block_frontier.h>
#include <block_topology.h>
#include <k2node.h>
#include <queries_state.h>
}

TEST(k2node_small_tests, test1) {

  auto value = 1UL << 63UL;
  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  TREE_DEPTH_T treedepth = 64;
  TREE_DEPTH_T cutdepth = 10;
  init_k2qstate(&st, treedepth, 256, cutdepth);

  int already_exists;
  k2node_insert_point(root_node, value, value, &st, &already_exists);

  struct pair2dl result_pair;

  k2node_scan_points_interactively(
      root_node, &st,
      [](unsigned long col, unsigned long row, void *rs) {
        auto &rp = *reinterpret_cast<struct pair2dl *>(rs);
        rp.col = col;
        rp.row = row;
      },
      &result_pair);

  ASSERT_EQ(result_pair.col, value);
  ASSERT_EQ(result_pair.row, value);

  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}

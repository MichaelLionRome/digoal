//
// Created by cristobal on 9/24/21.
//
#include <gtest/gtest.h>

extern "C" {
#include <k2node.h>
}
using vp_t = std::vector<std::pair<unsigned long, unsigned long>>;

TEST(k2node_problematic_input_tests2, full_scan_test) {

  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  TREE_DEPTH_T treedepth = 64;
  TREE_DEPTH_T cutdepth = 10;
  init_k2qstate(&st, treedepth, 256, cutdepth);

  unsigned long col = 710230858;
  unsigned long row = 4951110;

  int already_exists;
  k2node_insert_point(root_node, col, row, &st, &already_exists);

  struct k2node_lazy_handler_naive_scan_t lh;
  k2node_naive_scan_points_lazy_init(root_node, &st, &lh);

  std::vector<std::pair<unsigned long, unsigned long>> results_lazy;
  for (;;) {
    int has_next;
    k2node_naive_scan_points_lazy_has_next(&lh, &has_next);
    if (!has_next)
      break;

    pair2dl_t result;
    k2node_naive_scan_points_lazy_next(&lh, &result);
    results_lazy.emplace_back(result.col, result.row);
  }

  ASSERT_EQ(results_lazy.size(), 1);
  ASSERT_EQ(results_lazy[0].first, col);
  ASSERT_EQ(results_lazy[0].second, row);

  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}

TEST(k2node_problematic_input_tests2, band_row_scan) {

  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  TREE_DEPTH_T treedepth = 64;
  TREE_DEPTH_T cutdepth = 10;
  init_k2qstate(&st, treedepth, 256, cutdepth);

  unsigned long col = 710230858;
  unsigned long row = 4951110;

  int already_exists;
  k2node_insert_point(root_node, col, row, &st, &already_exists);

  struct k2node_lazy_handler_report_band_t lh;
  k2node_report_row_lazy_init(&lh, root_node, &st, row);

  std::vector<unsigned long> results_lazy;
  for (;;) {
    int has_next;
    k2node_report_band_has_next(&lh, &has_next);
    if (!has_next)
      break;

    uint64_t result;
    k2node_report_band_next(&lh, &result);
    results_lazy.push_back(result);
  }

  ASSERT_EQ(results_lazy.size(), 1);
  ASSERT_EQ(results_lazy[0], col);

  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}
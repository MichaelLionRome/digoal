//
// Created by cristobal on 5/24/21.
//

#include <gtest/gtest.h>

extern "C" {
#include <block.h>
#include <block_frontier.h>
#include <block_topology.h>
#include <k2node.h>
#include <queries_state.h>
}

#include "block_wrapper.hpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

TEST(lazy_scan_test, test_full_scan_1) {
  uint32_t treedepth = 30;
  BlockWrapper b(treedepth, 64);

  std::vector<std::pair<unsigned long, unsigned long>> expected_results;

  for (ulong i = 0; i < 10000; i++) {
    b.insert(i, i);
    expected_results.emplace_back(i, i);
  }
  struct lazy_handler_naive_scan_t lh;
  naive_scan_points_lazy_init(b.get_root(), b.get_qs(), &lh);

  std::vector<std::pair<unsigned long, unsigned long>> results_lazy;
  for (;;) {
    int has_next;
    naive_scan_points_lazy_has_next(&lh, &has_next);
    if (!has_next)
      break;

    pair2dl_t result;
    naive_scan_points_lazy_next(&lh, &result);
    results_lazy.emplace_back(result.col, result.row);
  }

  std::sort(results_lazy.begin(), results_lazy.end());

  ASSERT_EQ(results_lazy.size(), expected_results.size());
  ASSERT_EQ(results_lazy, expected_results);

  naive_scan_points_lazy_clean(&lh);
}

TEST(lazy_scan_test, test_band_1) {
  uint32_t treedepth = 30;
  BlockWrapper b(treedepth, 64);

  std::vector<unsigned long> expected_results;

  for (ulong i = 0; i < 10000; i++) {
    b.insert(129, i);
    expected_results.push_back(i);
  }
  struct lazy_handler_report_band_t lh;
  report_column_lazy_init(&lh, b.get_root(), b.get_qs(), 129);

  std::vector<unsigned long> results_lazy;
  for (;;) {
    int has_next;
    report_band_has_next(&lh, &has_next);
    if (!has_next)
      break;

    uint64_t result;
    report_band_next(&lh, &result);
    results_lazy.push_back(result);
  }

  std::sort(results_lazy.begin(), results_lazy.end());

  ASSERT_EQ(results_lazy.size(), expected_results.size());
  ASSERT_EQ(results_lazy, expected_results);

  report_band_lazy_clean(&lh);
}

TEST(lazy_scan_test, test_band_2) {
  for (unsigned int seed = 0; seed < 50; seed++) {
    uint32_t treedepth = 30;
    BlockWrapper b(treedepth, 64);

    std::vector<unsigned long> expected_results;
    std::vector<unsigned long> shuffling_coords;

    for (unsigned long i = 0; i < static_cast<unsigned long>(1e5); i++) {
      shuffling_coords.push_back(i);
    }
    srand(seed);
    std::shuffle(shuffling_coords.begin(), shuffling_coords.end(),
                 std::mt19937(std::random_device()()));

    std::vector<unsigned long> selected_coords(
        shuffling_coords.begin(),
        shuffling_coords.begin() + static_cast<unsigned long>(1e3));

    const auto col_value = shuffling_coords.back();

    for (auto coord : selected_coords) {
      b.insert(col_value, coord);
      expected_results.push_back(coord);
    }
    struct lazy_handler_report_band_t lh;
    report_column_lazy_init(&lh, b.get_root(), b.get_qs(), col_value);

    std::vector<unsigned long> results_lazy;
    for (;;) {
      int has_next;
      report_band_has_next(&lh, &has_next);
      if (!has_next)
        break;

      uint64_t result;
      report_band_next(&lh, &result);
      results_lazy.push_back(result);
    }
    std::sort(expected_results.begin(), expected_results.end());
    std::sort(results_lazy.begin(), results_lazy.end());

    ASSERT_EQ(results_lazy.size(), expected_results.size());
    ASSERT_EQ(results_lazy, expected_results);

    report_band_lazy_clean(&lh);
  }
}

TEST(lazy_scan_test, test_k2node_full_scan_1) {
  TREE_DEPTH_T treedepth = 32;
  TREE_DEPTH_T cut_depth = 10;

  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  init_k2qstate(&st, treedepth, 255, cut_depth);

  std::vector<std::pair<unsigned long, unsigned long>> expected_results;

  int already_exists;
  for (ulong i = 0; i < 10000; i++) {
    k2node_insert_point(root_node, i, i, &st, &already_exists);
    expected_results.emplace_back(i, i);
  }
  // naive_scan_points_lazy_init(b.get_root(), b.get_qs(), &lh);
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

  std::sort(results_lazy.begin(), results_lazy.end());
  ASSERT_GE(results_lazy.at(0).first, 0);
  ASSERT_EQ(results_lazy.size(), expected_results.size());
  ASSERT_EQ(results_lazy, expected_results);

  k2node_naive_scan_points_lazy_clean(&lh);
  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}

TEST(lazy_scan_test, test_k2node_band_scan_1) {
  TREE_DEPTH_T treedepth = 32;
  TREE_DEPTH_T cut_depth = 10;

  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  init_k2qstate(&st, treedepth, 255, cut_depth);

  const unsigned long row_const_value = 1239;

  std::vector<unsigned long> expected_results;

  int already_exists;
  for (ulong i = 0; i < 10000; i++) {
    k2node_insert_point(root_node, i, row_const_value, &st, &already_exists);
    expected_results.push_back(i);
  }
  // naive_scan_points_lazy_init(b.get_root(), b.get_qs(), &lh);
  struct k2node_lazy_handler_report_band_t lh;
  k2node_report_row_lazy_init(&lh, root_node, &st, row_const_value);

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

  std::sort(results_lazy.begin(), results_lazy.end());
  ASSERT_EQ(results_lazy.size(), expected_results.size());
  ASSERT_EQ(results_lazy, expected_results);

  k2node_report_band_lazy_clean(&lh);
  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}

/*
MIT License

Copyright (c) 2020 Cristobal Miranda T.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <algorithm>
#include <gtest/gtest.h>
#include <iostream>
#include <utility>
#include <vector>

extern "C" {
#include <block.h>
#include <block_frontier.h>
#include <block_topology.h>
#include <k2node.h>
#include <queries_state.h>
}

TEST(k2node_tests, problematic_input_seq_1_test) {
  std::vector<std::pair<unsigned long, unsigned long>> pairs = {
      {2435942, 1822678}, {2462421, 1920115}, {2366760, 1595921},
      {2117488, 1141805}, {2113889, 1131953}, {2385134, 1624246},
      {2408969, 1678218}, {2335783, 1541134}, {2449482, 1876875},
      {2436055, 1823182}, {2474962, 1960901}, {2037784, 960877},
      {2436135, 1823442}, {2012038, 896833},  {2417018, 1717381},
      {2413589, 1701903}, {2462344, 1919937}, {2367773, 1597498},
      {2430360, 1788087}, {2437050, 1826803}, {2419887, 1736674},
      {2328978, 1531403}, {2196922, 1299801}, {2431635, 1794752},
      {2210236, 1323004}, {2444318, 1854880}, {2339694, 1546957},

  };

  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  TREE_DEPTH_T treedepth = 32;
  TREE_DEPTH_T cutdepth = 10;
  init_k2qstate(&st, treedepth, 256, cutdepth);

  int already_exists;
  int i = 0;

  for (const auto &pair : pairs) {
    if (i == (int)pairs.size() - 1) {
      already_exists = 0; // debug point
    }
    k2node_insert_point(root_node, pair.first, pair.second, &st,
                        &already_exists);
    i++;
  }

  ASSERT_EQ(debug_validate_k2node_rec(root_node, &st, 0), 0);

  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}

TEST(k2node_tests, can_traverse_column) {

  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  TREE_DEPTH_T treedepth = 32;
  TREE_DEPTH_T cutdepth = 10;
  init_k2qstate(&st, treedepth, 255, cutdepth);

  int already_exists;

  unsigned long col_choice = 123123;
  for (unsigned long row = 123123; row < 123123 + 1000; row++) {
    k2node_insert_point(root_node, col_choice, row, &st, &already_exists);
  }

  struct vector_pair2dl_t result;
  vector_pair2dl_t__init_vector(&result);
  k2node_report_column(root_node, col_choice, &st, &result);

  ASSERT_EQ(result.nof_items, 1000);
  for (unsigned long i = 0; i < 1000; i++) {
    ASSERT_EQ(result.data[i].row, 123123 + i) << "Failed at i=" << i;
  }

  ASSERT_EQ(debug_validate_k2node_rec(root_node, &st, 0), 0);

  vector_pair2dl_t__free_vector(&result);
  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}

void can_traverse_column_interactively_fun(unsigned long col, unsigned long row,
                                           void *report_state) {
  struct vector_pair2dl_t *result = (struct vector_pair2dl_t *)report_state;
  struct pair2dl pair;
  pair.col = col;
  pair.row = row;
  vector_pair2dl_t__insert_element(result, pair);
}

TEST(k2node_tests, can_traverse_column_interactively) {

  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  TREE_DEPTH_T treedepth = 32;
  TREE_DEPTH_T cutdepth = 10;
  init_k2qstate(&st, treedepth, 255, cutdepth);

  int already_exists;

  unsigned long col_choice = 123123;
  for (unsigned long row = 123123; row < 123123 + 1000; row++) {
    k2node_insert_point(root_node, col_choice, row, &st, &already_exists);
  }

  struct vector_pair2dl_t result;
  vector_pair2dl_t__init_vector(&result);
  k2node_report_column_interactively(root_node, col_choice, &st,
                                     can_traverse_column_interactively_fun,
                                     &result);

  ASSERT_EQ(result.nof_items, 1000);
  for (unsigned long i = 0; i < 1000; i++) {
    ASSERT_EQ(result.data[i].row, 123123 + i) << "Failed at i=" << i;
  }

  ASSERT_EQ(debug_validate_k2node_rec(root_node, &st, 0), 0);
  vector_pair2dl_t__free_vector(&result);
  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}

TEST(k2node_tests, can_insert_1) {

  for (TREE_DEPTH_T treedepth = 3; treedepth < 7; treedepth++) {
    for (TREE_DEPTH_T cut_depth = 1; cut_depth < treedepth - 1; cut_depth++) {
      struct k2node *root_node = create_k2node();

      struct k2qstate st;
      init_k2qstate(&st, treedepth, 255, cut_depth);

      unsigned long side = 1 << treedepth;

      int already_exists;
      for (unsigned long col = 0; col < side; col++) {
        for (unsigned long row = 0; row < side; row++) {
          k2node_insert_point(root_node, col, row, &st, &already_exists);
          for (unsigned long col_check = 0; col_check < col; col_check++) {
            for (unsigned long row_check = 0; row_check < row; row_check++) {
              int has_the_point;
              k2node_has_point(root_node, col_check, row_check, &st,
                               &has_the_point);
              ASSERT_TRUE(has_the_point)
                  << (int)treedepth << ";" << (int)cut_depth
                  << " (1)- current (" << col << ", " << row << ") - check: ("
                  << col_check << ", " << row_check << ")";
            }
          }
          for (unsigned long row_check = 0; row_check <= row; row_check++) {
            int has_the_point;
            k2node_has_point(root_node, col, row_check, &st, &has_the_point);
            ASSERT_TRUE(has_the_point)
                << (int)treedepth << ";" << (int)cut_depth << " (2)- current ("
                << col << ", " << row << ") - check: (" << col << ", "
                << row_check << ")";
          }

          for (unsigned long col_check = col + 1; col_check < side;
               col_check++) {
            for (unsigned long row_check = row + 1; row_check < side;
                 row_check++) {
              int has_the_point;
              k2node_has_point(root_node, col_check, row_check, &st,
                               &has_the_point);
              ASSERT_FALSE(has_the_point)
                  << (int)treedepth << ";" << (int)cut_depth
                  << " (3)- current (" << col << ", " << row << ") - check: ("
                  << col_check << ", " << row_check << ")";
            }
          }
          for (unsigned long row_check = row + 1; row_check < side;
               row_check++) {
            int has_the_point;
            k2node_has_point(root_node, col, row_check, &st, &has_the_point);
            if ((bool)has_the_point) {
              struct vector_pair2dl_t result;

              vector_pair2dl_t__init_vector(&result);

              k2node_naive_scan_points(root_node, &st, &result);

              for (int i = 0; i < result.nof_items; i++) {
                printf("(%lu, %lu) ", result.data[i].col, result.data[i].row);
              }
              printf("\n");

              vector_pair2dl_t__free_vector(&result);
            }
            ASSERT_FALSE((bool)has_the_point)
                << (int)treedepth << ";" << (int)cut_depth << " (4)- current ("
                << col << ", " << row << ") - check: (" << col << ", "
                << row_check << ")";
          }
        }
      }

      ASSERT_EQ(debug_validate_k2node_rec(root_node, &st, 0), 0);

      free_rec_k2node(root_node, 0, st.cut_depth);
      clean_k2qstate(&st);
    }
  }
}

TEST(k2node_tests, report_band_lazy_test_1) {

  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  TREE_DEPTH_T treedepth = 32;
  TREE_DEPTH_T cutdepth = 10;
  init_k2qstate(&st, treedepth, 255, cutdepth);

  int already_exists;

  std::set<std::pair<unsigned long, unsigned long>> init_elements = {
      {0, 0},  {3, 3}, {15, 3}, {3, 15}, {30, 31}, {31, 8},
      {3, 30}, {3, 2}, {3, 29}, {8, 15}, {9, 15},  {15, 15}};

  std::set<unsigned long> expected_values = {3, 8, 9, 15};

  for (auto &p : init_elements)
    k2node_insert_point(root_node, p.first, p.second, &st, &already_exists);

  k2node_lazy_handler_report_band_t lh;
  k2node_report_row_lazy_init(&lh, root_node, &st, 15);

  std::set<unsigned long> values;
  std::set<unsigned long> values2;

  int has_next = TRUE;
  for (;;) {
    k2node_report_band_has_next(&lh, &has_next);
    if (!has_next)
      break;
    uint64_t result;
    k2node_report_band_next(&lh, &result);
    values.insert(result);
  }

  ASSERT_EQ(values, expected_values);

  k2node_report_band_reset(&lh);
  for (;;) {
    k2node_report_band_has_next(&lh, &has_next);
    if (!has_next)
      break;
    uint64_t result;
    k2node_report_band_next(&lh, &result);
    values2.insert(result);
  }

  ASSERT_EQ(values2, expected_values);

  k2node_report_band_lazy_clean(&lh);
  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}

TEST(k2node_tests, report_all_lazy_test_1) {

  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  TREE_DEPTH_T treedepth = 32;
  TREE_DEPTH_T cutdepth = 10;
  init_k2qstate(&st, treedepth, 255, cutdepth);

  int already_exists;

  std::set<std::pair<unsigned long, unsigned long>> init_elements = {
      {0, 0},  {3, 3}, {15, 3}, {3, 15}, {30, 31}, {31, 8},
      {3, 30}, {3, 2}, {3, 29}, {8, 15}, {9, 15},  {15, 15}};

  for (auto &p : init_elements)
    k2node_insert_point(root_node, p.first, p.second, &st, &already_exists);

  k2node_lazy_handler_naive_scan_t lh;

  k2node_naive_scan_points_lazy_init(root_node, &st, &lh);

  std::set<std::pair<unsigned long, unsigned long>> values;
  std::set<std::pair<unsigned long, unsigned long>> values2;

  int has_next = TRUE;
  for (;;) {
    k2node_naive_scan_points_lazy_has_next(&lh, &has_next);
    if (!has_next)
      break;

    struct pair2dl result;
    k2node_naive_scan_points_lazy_next(&lh, &result);
    values.insert({result.col, result.row});
  }

  ASSERT_EQ(values, init_elements);

  k2node_naive_scan_points_lazy_reset(&lh);

  for (;;) {
    k2node_naive_scan_points_lazy_has_next(&lh, &has_next);
    if (!has_next)
      break;

    struct pair2dl result;
    k2node_naive_scan_points_lazy_next(&lh, &result);
    values2.insert({result.col, result.row});
  }

  ASSERT_EQ(values2, init_elements);

  for (int i = 0; i < (int)init_elements.size(); i++) {
    std::set<std::pair<unsigned long, unsigned long>> values3;
    for (int j = 0; j < i; j++) {
      k2node_naive_scan_points_lazy_has_next(&lh, &has_next);
      if (!has_next)
        break;

      struct pair2dl result;
      k2node_naive_scan_points_lazy_next(&lh, &result);
      values3.insert({result.col, result.row});
    }
    k2node_naive_scan_points_lazy_reset(&lh);

    ASSERT_EQ(values3.size(), i);
    for (auto &p : values3) {
      ASSERT_TRUE(init_elements.find(p) != init_elements.end());
    }
  }

  k2node_naive_scan_points_lazy_clean(&lh);
  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}

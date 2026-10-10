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
#include <gtest/gtest.h>

extern "C" {
#include <block.h>
#include <block_frontier.h>
#include <block_topology.h>
#include <k2node.h>
#include <queries_state.h>
}

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

TEST(k2node_tests, can_insert_1) {

  for (TREE_DEPTH_T treedepth = 3; treedepth < 7; treedepth++) {
    for (TREE_DEPTH_T cut_depth = 1; cut_depth < treedepth - 1; cut_depth++) {
      struct k2node *root_node = create_k2node();

      struct k2qstate st;
      init_k2qstate(&st, treedepth, 255, cut_depth);

      ulong side = 1 << treedepth;

      int already_exists;
      for (ulong col = 0; col < side; col++) {
        for (ulong row = 0; row < side; row++) {
          k2node_insert_point(root_node, col, row, &st, &already_exists);
          for (ulong col_check = 0; col_check < col; col_check++) {
            for (ulong row_check = 0; row_check < row; row_check++) {
              int has_the_point;
              k2node_has_point(root_node, col_check, row_check, &st,
                               &has_the_point);
              ASSERT_TRUE(has_the_point)
                  << (int)treedepth << ";" << (int)cut_depth
                  << " (1)- current (" << col << ", " << row << ") - check: ("
                  << col_check << ", " << row_check << ")";
            }
          }
          for (ulong row_check = 0; row_check <= row; row_check++) {
            int has_the_point;
            k2node_has_point(root_node, col, row_check, &st, &has_the_point);
            ASSERT_TRUE(has_the_point)
                << (int)treedepth << ";" << (int)cut_depth << " (2)- current ("
                << col << ", " << row << ") - check: (" << col << ", "
                << row_check << ")";
          }

          for (ulong col_check = col + 1; col_check < side; col_check++) {
            for (ulong row_check = row + 1; row_check < side; row_check++) {
              int has_the_point;
              k2node_has_point(root_node, col_check, row_check, &st,
                               &has_the_point);
              ASSERT_FALSE(has_the_point)
                  << (int)treedepth << ";" << (int)cut_depth
                  << " (3)- current (" << col << ", " << row << ") - check: ("
                  << col_check << ", " << row_check << ")";
            }
          }
          for (ulong row_check = row + 1; row_check < side; row_check++) {
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

      free_rec_k2node(root_node, 0, st.cut_depth);
      clean_k2qstate(&st);
    }
  }
}
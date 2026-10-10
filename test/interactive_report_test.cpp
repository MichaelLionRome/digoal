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
#include <queries_state.h>
}

#include <algorithm>
#include <iostream>
#include <vector>

bool has_pair(struct vector_pair2dl_t *v, long col, long row) {
  for (int i = 0; i < v->nof_items; i++) {
    struct pair2dl pair = v->data[i];
    if (col == pair.col && row == pair.row) {
      return true;
    }
  }
  return false;
}

void print_vector(struct vector_pair2dl_t *v) {
  for (int i = 0; i < v->nof_items; i++) {
    struct pair2dl pair = v->data[i];
    std::cout << "(" << pair.col << ", " << pair.row << ")" << std::endl;
  }
}

void interactive_report_row(unsigned long column, unsigned long row,
                            void *report_state) {
  struct vector_pair2dl_t *v = (struct vector_pair2dl_t *)report_state;
  struct pair2dl pair;
  pair.row = row;
  pair.col = column;
  vector_pair2dl_t__insert_element(v, pair);
  std::cout << "found pair (" << column << ", " << row << ")" << std::endl;
}

void interactive_print_all(unsigned long column, unsigned long row, void *) {
  std::cout << "(" << column << ", " << row << ")" << std::endl;
}

TEST(interactive_block_test, test1) {
  int err_code;
  uint32_t treedepth = 5;
  struct block *root_block = create_block();

  struct queries_state qs;
  init_queries_state(&qs, treedepth, MAX_NODES_IN_BLOCK, root_block);

  std::vector<struct pair2dl> init_elements = {
      {0, 0},  {3, 3}, {15, 3}, {3, 15}, {30, 31}, {31, 8},
      {3, 30}, {3, 2}, {3, 29}, {8, 15}, {9, 15},  {15, 15}};

  int qty = init_elements.size();
  int already_exists;
  for (int i = 0; i < qty; i++) {
    insert_point(root_block, init_elements[i].col, init_elements[i].row, &qs,
                 &already_exists);
  }

  struct vector_pair2dl_t result;
  vector_pair2dl_t__init_vector_with_capacity(&result, 10);

  err_code = report_row_interactively(root_block, 15, &qs,
                                      interactive_report_row, &result);

  if (err_code) {
    exit(err_code);
  }

  std::cout << "printing vector" << std::endl;
  print_vector(&result);

  ASSERT_TRUE(has_pair(&result, 3, 15)) << "Cant find pair 3,15";
  ASSERT_TRUE(has_pair(&result, 8, 15)) << "Cant find pair 8,15";
  ASSERT_TRUE(has_pair(&result, 9, 15)) << "Cant find pair 9,15";
  ASSERT_TRUE(has_pair(&result, 15, 15)) << "Cant find pair 15,15";

  ASSERT_EQ(result.nof_items, 4) << "Does not have 4 elements";

  std::cout << "now printing all points..." << std::endl;

  scan_points_interactively(root_block, &qs, interactive_print_all, nullptr);

  vector_pair2dl_t__free_vector(&result);

  finish_queries_state(&qs);
  free_rec_block(root_block);
}

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

extern "C" {
#include <block.h>
#include <block_frontier.h>
#include <block_topology.h>
#include <queries_state.h>
}

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include <gtest/gtest.h>

TEST(insertion_out_of_bounds, test_1) {
  uint32_t tree_depth = 16;
  struct block *root_block = create_block();
  struct queries_state qs;
  init_queries_state(&qs, tree_depth, MAX_NODES_IN_BLOCK, root_block);

  std::ifstream ifstream("../points_output.txt");

  if (!ifstream.is_open()) {
    FAIL() << "NOT OPEN";
  }

  unsigned long col, row;
  while (ifstream >> col >> row) {
    std::cout << col << ", " << row << std::endl;
    int already_exists;
    insert_point(root_block, col, row, &qs, &already_exists);
    int has_it;
    has_point(root_block, col, row, &qs, &has_it);
    ASSERT_TRUE(has_it);
  }
}

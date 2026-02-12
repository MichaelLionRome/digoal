
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
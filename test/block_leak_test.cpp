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

#include "block_wrapper.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

int main() {
  {
    int seed = 0;
    uint32_t treedepth = 5;
    srand(seed);
    ulong side = 1u << treedepth;
    ulong matrix_size = side * side;
    BlockWrapper b(treedepth, 16);

    std::vector<ulong> indexes(matrix_size, 0);

    for (ulong i = 0; i < matrix_size; i++)
      indexes[i] = i;

    random_shuffle(indexes.begin(), indexes.end());

    for (ulong i = 0; i < matrix_size; i++) {
      ulong col = i / side;
      ulong row = i % side;
      b.insert(col, row);
    }
  }

  exit(0);
}
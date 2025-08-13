//
// Created by Cristobal Miranda, 2020
//

extern "C" {
#include <block.h>
#include <block_topology.h>
#include <block_frontier.h>
#include <queries_state.h>
}

#include "block_wrapper.hpp"

#include <iostream>
#include <algorithm>
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

    for (ulong i = 0; i < matrix_size; i++) indexes[i] = i;

    random_shuffle(indexes.begin(), indexes.end());

    for (ulong i = 0; i < matrix_size; i++) {
      ulong col = i / side;
      ulong row = i % side;
      b.insert(col, row);
    }
  }


  exit(0);
}
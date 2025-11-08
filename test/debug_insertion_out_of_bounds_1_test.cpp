//
// Created by Cristobal Miranda, 2020
//


extern "C" {
#include <block.h>
#include <block_topology.h>
#include <block_frontier.h>
#include <queries_state.h>
}

#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>


#include <gtest/gtest.h>


TEST(insertion_out_of_bounds, test_1){
  uint32_t tree_depth = 16;
  struct block *root_block = create_block(tree_depth);
  struct queries_state qs;
  init_queries_state(&qs, tree_depth);




  std::ifstream ifstream("../points_output.txt");

  if(!ifstream.is_open()){
    FAIL() << "NOT OPEN";
  }

  ulong col, row;
  while(ifstream >> col >> row){
    std::cout << col << ", " << row << std::endl;
    if(col == 19469 && row == 1072){
      int debug = 0;
    }
    insert_point(root_block, col, row, &qs);
    int has_it;
    has_point(root_block, col, row, &qs, &has_it);
    ASSERT_TRUE(has_it);
  }



}

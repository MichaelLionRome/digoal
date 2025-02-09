#include <gtest/gtest.h>

extern "C" {
#include "block/block.h"
#include "block-topology/block_topology.h"
#include "block-frontier/block_frontier.h"
#include "block/queries_state.h"
}

#include "block_wrapper.hpp"

#include <iostream>

using namespace std;

TEST(block_test, test1){
    uint32_t tree_depth = 3;
    struct block *root_block = create_block(tree_depth);
    struct queries_state qs;
    init_queries_state(&qs, tree_depth);


    insert_point(root_block, 0, 0, &qs);
    int found_point;
    has_point(root_block, 0, 0, &qs, &found_point);
    ASSERT_EQ(1, found_point);

    free_block(root_block);
    finish_queries_state(&qs);
}

TEST(block_test, test2){
  uint32_t treedepth = 3;
  ulong side = 1 << treedepth;
  BlockWrapper b(treedepth);
  b.insert(0, 0);
  b.insert(0, 1);
  b.insert(1, 0);
  ASSERT_TRUE(b.has(0, 0)) << "Block does not have point 0, 0";
  ASSERT_TRUE(b.has(0, 1)) << "Block does not have point 0, 1";
  ASSERT_TRUE(b.has(1, 0)) << "Block does not have point 1, 0";
  for(ulong col = 0; col < side; col++){
    for(ulong row = 0; row < side; row++){
      if((col == 0 && row == 0) || (col == 0 && row == 1) || (col == 1 && row == 0)){
        continue;
      }
      ASSERT_FALSE(b.has(col, row)) << "Block has point " << col << ", " << row;
    }
  }
}

TEST(block_test, fills_depth_3){
  uint32_t treedepth = 3;
  ulong side = 1 << treedepth;
  BlockWrapper b(treedepth);

  for(ulong col = 0; col < side; col++){
    for(ulong row = 0; row < side; row++){
      cout << "col = " << col << ", row = " << row << endl;
      if(col == 2 && row == 0){
        int debug = 0;
      }
      b.insert(col, row);
      for(ulong col_check = 0; col_check < col; col_check++){
        for(ulong row_check = 0; row_check < row; row_check++){
          ASSERT_TRUE(b.has(col_check, row_check)) << "at (" << col << ", " << row <<  ") Must have point " << col_check << ", " << row_check;
        }
      }
      for(ulong row_check = 0; row_check <= row; row_check++){
        ASSERT_TRUE(b.has(col, row_check)) << "at (" << col << ", " << row <<  ") Must have point " << col << ", " << row_check;
      }
      for(ulong row_check = row+1; row_check < side; row_check++){
        ASSERT_FALSE(b.has(col, row_check)) << "at (" << col << ", " << row <<  ") Must not have point " << col << ", " << row_check;
      }
      for(ulong col_check = col+1; col_check < side; col_check++){
        for(ulong row_check = 0; row_check < side; row_check++){
          ASSERT_FALSE(b.has(col_check, row_check)) << "at (" << col << ", " << row <<  ") Must not have point " << col_check << ", " << row_check;
        }
      }
    }
  }
  for(int col = 0; col < side; col++){
    for(int row = 0; row < side; row++){
      ASSERT_TRUE(b.has(col, row)) << "No point " << col << ", " << row;
    }
  }
}

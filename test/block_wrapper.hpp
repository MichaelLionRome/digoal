//
// Created by Cristobal Miranda, 2020
//

#ifndef K2TREEELEMENTSTEST_BLOCK_WRAPPER_HPP
#define K2TREEELEMENTSTEST_BLOCK_WRAPPER_HPP

extern "C" {
#include "block/block.h"
#include "block-topology/block_topology.h"
#include "block-frontier/block_frontier.h"
#include "block/queries_state.h"
}

#include <stdexcept>
#include <string>

class BlockWrapper{
  struct block *b;
  struct queries_state qs;

public:
  BlockWrapper(uint32_t tree_depth){
    b = create_block(tree_depth);
    init_queries_state(&qs, tree_depth);
  }

  ~BlockWrapper(){
    free_block(b);
    finish_queries_state(&qs);
  }

  void insert(unsigned long col, unsigned long row){
    insert_point(b, col, row, &qs);
  }

  bool has(unsigned long col, unsigned long row){
    int result;
    has_point(b, col, row, &qs, &result);
    return result == 1;
  }
};

#endif //K2TREEELEMENTSTEST_BLOCK_WRAPPER_HPP

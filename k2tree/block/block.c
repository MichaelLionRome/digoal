#include <bitvector.h>

#include "block.h"
#include "block-frontier/block_frontier.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-variable"

struct child_result {
  struct block *resulting_block;
  uint32_t resulting_node_idx;
  uint32_t resulting_depth;
  int is_leaf_result;
};

int child(struct block *input_block, uint32_t input_node_idx,
          uint32_t requested_child_position, uint32_t input_node_relative_depth,
          struct child_result *result);

/**
  Posible return codes:
  SUCCESS_CODE: child was found and stored in output variables
  DOES_NOT_EXIST_CHILD_ERR: child doesn't exist and output variables don't hold
any data
**/
int child(struct block *input_block, uint32_t input_node_idx,
          uint32_t requested_child_position, uint32_t input_node_relative_depth,
          struct child_result *result) {
  uint32_t tree_depth = input_block->tree_depth;
  if (input_node_relative_depth + 1 == tree_depth) {
    /* Create leaf result */
    result->resulting_block = input_block;
    result->resulting_node_idx = input_node_idx;
    result->resulting_depth = input_node_relative_depth;
    result->is_leaf_result = TRUE;
    return SUCCESS_ECODE;
  }

  int exists;
  CHECK_ERR(child_exists(input_block->bt, input_node_idx,
                         requested_child_position, &exists));
  if (!exists)
    return DOES_NOT_EXIST_CHILD_ERR;

  int frontier_traversal_idx = 0;
  int is_frontier;
  CHECK_ERR(frontier_check(input_block->bf, input_node_idx, &frontier_traversal_idx, &is_frontier));

  if(is_frontier){
    struct block * child_block;
    get_child_block(input_block->bf, frontier_traversal_idx, &child_block);
    return child(child_block, 0, requested_child_position, 0, result);
  }

  

  return NOT_IMPLEMENTED;
}

int has_point(struct block *input_block, uint32_t col, uint32_t row) {
  return 0;
}

int insert_point(struct block *input_block, uint32_t col, uint32_t row) {
  return 0;
}

#pragma clang diagnostic pop

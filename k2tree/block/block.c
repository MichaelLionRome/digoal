#include <bitvector.h>

#include "block-frontier/block_frontier.h"
#include "block.h"
#include "definitions.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-variable"

struct child_result {
  struct block *resulting_block;
  uint32_t resulting_node_idx;
  uint32_t resulting_relative_depth;
  int is_leaf_result;
};

struct sequential_scan_result {
  uint32_t child_preorder;
  uint32_t node_relative_depth;
  struct vector *subtrees_count_map;
  struct vector *relative_depth_map;
};

uint32_t skip_table[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
                         0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 2, 0, 0, 1, 2,
                         0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 2, 0, 1, 1, 2,
                         0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 3, 0, 1, 2, 3};

static inline uint32_t get_subtree_skipping_qty(struct block *b,
                                                uint32_t node_idx,
                                                uint32_t child_idx) {
  uint32_t node_value;
  SAFE_OP(read_node(b->bt, node_idx, node_value));
  return skip_table[4 * node_value + child_idx];
}

/* PROTOTYPES */
int sequential_scan_child(struct block *input_block, uint32_t input_node_idx,
                      uint32_t subtrees_to_skip,
                      uint32_t *frontier_traversal_idx,
                      uint32_t input_node_relative_depth,
                      struct sequential_scan_result *result);

/**
  Posible return codes:
  SUCCESS_CODE: child was found and stored in output variables
  DOES_NOT_EXIST_CHILD_ERR: child doesn't exist and output variables don't hold
any data
**/
int child(struct block *input_block, uint32_t input_node_idx,
          uint32_t requested_child_position, uint32_t input_node_relative_depth,
          struct child_result *result);
/* END PROTOTYPES */



int child(struct block *input_block, uint32_t input_node_idx,
          uint32_t requested_child_position, uint32_t input_node_relative_depth,
          struct child_result *result) {
  uint32_t tree_depth = input_block->tree_depth;
  if (input_node_relative_depth + 1 == tree_depth) {
    /* Create leaf result */
    result->resulting_block = input_block;
    result->resulting_node_idx = input_node_idx;
    result->resulting_relative_depth = input_node_relative_depth;
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
  CHECK_ERR(frontier_check(input_block->bf, input_node_idx,
                           &frontier_traversal_idx, &is_frontier));

  if (is_frontier) {
    struct block *child_block;
    get_child_block(input_block->bf, frontier_traversal_idx, &child_block);
    return child(child_block, 0, requested_child_position, 0, result);
  }

  uint32_t subtrees_to_skip = get_subtree_skipping_qty(
      input_block, input_node_idx, requested_child_position);
  struct sequential_scan_result sc_result;
  SAFE_OP(sequential_scan_child(input_block, input_node_idx, subtrees_to_skip,
                        &frontier_traversal_idx, input_node_relative_depth,
                        &sc_result));
  // seq_scan_result_cleanup(sc_result);

  result->resulting_block = input_block;
  result->resulting_node_idx = sc_result.child_preorder + 1;
  result->resulting_relative_depth = input_node_relative_depth + 1;

  return SUCCESS_ECODE;
}

int has_point(struct block *input_block, uint32_t col, uint32_t row) {
  return 0;
}

int insert_point(struct block *input_block, uint32_t col, uint32_t row) {
  return 0;
}

#pragma clang diagnostic pop

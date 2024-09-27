#include "block.h"
#include <bitvector.h>

#define CHECK_ERR(err)                                                         \
  do {                                                                         \
    if (err) {                                                                 \
      return (err);                                                            \
    }                                                                          \
  } while (0)

#define SUCCESS_ECODE 0;
#define DOES_NOT_EXIST_CHILD_ERR 1;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

/**
  Possible return codes:
  SUCCESS_CODE: Whether child exists or not was determined
**/
static inline int child_exists(struct block *input_block,
                               uint32_t input_node_idx,
                               uint32_t requested_child_position,
                               uint32_t *result) {
  struct bitvector *bv = input_block->bt->bv;
  int bit_on;
  CHECK_ERR(
      bit_read(bv, input_node_idx * 4 + requested_child_position, &bit_on));
  *result = bit_on;
  return SUCCESS_ECODE;
}
/**
  Posible return codes:
  SUCCESS_CODE: child was found and stored in output variables
  DOES_NOT_EXIST_CHILD_ERR: child doesn't exist and output variables don't hold
any data
**/
int child(
    /* Input values */
    struct block *input_block, uint32_t input_node_idx,
    uint32_t requested_child_position, uint32_t input_node_relative_depth,
    /* Output values */
    struct block **resulting_block, uint32_t *resulting_node_idx,
    uint32_t *resulting_depth, int *is_leaf_result) {
  int tree_depth = input_block->tree_depth;
  if (input_node_relative_depth + 1 == tree_depth) {
    /* Create leaf result */
    *resulting_block = input_block;
    *resulting_node_idx = input_node_idx;
    *resulting_depth = input_node_relative_depth;
    *is_leaf_result = 1;
    return SUCCESS_ECODE;
  }

  int exists;
  CHECK_ERR(child_exists(input_block, input_node_idx, requested_child_position,
                         &exists));
  if (!exists)
    return DOES_NOT_EXIST_CHILD_ERR;

  int frontier_traversal_idx = 0;
}

int has_point(struct block *input_block, uint32_t col, uint32_t row) {
  return 0;
}

int insert_point(struct block *input_block, uint32_t col, uint32_t row) {
  return 0;
}

#pragma clang diagnostic pop

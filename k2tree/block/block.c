#include "block.h"
#include <bitvector.h>

#define CHECK_ERR(err)                                                         \
  do {                                                                         \
    if (err) {                                                                 \
      return (err);                                                            \
    }                                                                          \
  } while (0)

#ifndef SUCCESS_ECODE
#define SUCCESS_ECODE 0;
#endif
#define DOES_NOT_EXIST_CHILD_ERR 1;

#define NOT_IMPLEMENTED -100

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic push
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
  Possible return codes:
  SUCCESS_CODE: Whether child exists or not was determined
**/
static inline int child_exists(struct block *input_block,
                               uint32_t input_node_idx,
                               uint32_t requested_child_position, int *result) {
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
  CHECK_ERR(child_exists(input_block, input_node_idx, requested_child_position,
                         &exists));
  if (!exists)
    return DOES_NOT_EXIST_CHILD_ERR;

  int frontier_traversal_idx = 0;

  return NOT_IMPLEMENTED;
}

int has_point(struct block *input_block, uint32_t col, uint32_t row) {
  return 0;
}

int insert_point(struct block *input_block, uint32_t col, uint32_t row) {
  return 0;
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop

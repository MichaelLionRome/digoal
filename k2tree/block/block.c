#include <bitvector.h>
#include <circular_stack.h>

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

struct node_subtree_info {
  uint32_t node_index;
  uint32_t node_relative_depth;
  uint32_t subtree_size;
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

static inline int mark_subtree_size(struct sequential_scan_result *sc_result,
                                    uint32_t node_index,
                                    uint32_t node_relative_depth,
                                    uint32_t subtree_size) {
  SAFE_OP(set_element_at(sc_result->subtrees_count_map, (char *)&subtree_size,
                         node_index));
  SAFE_OP(set_element_at(sc_result->relative_depth_map,
                         (char *)&node_relative_depth, node_index));

  return SUCCESS_ECODE;
}

/* INTERNAL FUNCTIONS PROTOTYPES */
int sequential_scan_child(struct block *input_block, uint32_t input_node_idx,
                          uint32_t subtrees_to_skip,
                          uint32_t *frontier_traversal_idx,
                          uint32_t input_node_relative_depth,
                          struct sequential_scan_result *result,
                          struct circular_stack *not_yet_traversed,
                          struct circular_stack *subtrees_count);

/**
  Posible return codes:
  SUCCESS_CODE: child was found and stored in output variables
  DOES_NOT_EXIST_CHILD_ERR: child doesn't exist and output variables don't hold
**/
int child(struct block *input_block, uint32_t input_node_idx,
          uint32_t requested_child_position, uint32_t input_node_relative_depth,
          struct child_result *result, struct sequential_scan_result *sc_result,
          struct circular_stack *not_yet_traversed,
          struct circular_stack *subtrees_count);
/* END INTERNAL FUNCTIONS  PROTOTYPES */

/* INTERNAL FUNCTIONS IMPLEMENTATIONS */

int child(struct block *input_block, uint32_t input_node_idx,
          uint32_t requested_child_position, uint32_t input_node_relative_depth,
          struct child_result *result, struct sequential_scan_result *sc_result,
          struct circular_stack *not_yet_traversed,
          struct circular_stack *subtrees_count) {
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
    return child(child_block, 0, requested_child_position, 0, result, sc_result,
                 not_yet_traversed, subtrees_count);
  }

  uint32_t subtrees_to_skip = get_subtree_skipping_qty(
      input_block, input_node_idx, requested_child_position);
  SAFE_OP(sequential_scan_child(
      input_block, input_node_idx, subtrees_to_skip, &frontier_traversal_idx,
      input_node_relative_depth, sc_result, not_yet_traversed, subtrees_count));

  result->resulting_block = input_block;
  result->resulting_node_idx = sc_result->child_preorder + 1;
  result->resulting_relative_depth = input_node_relative_depth + 1;

  return SUCCESS_ECODE;
}

int sequential_scan_child(struct block *input_block, uint32_t input_node_idx,
                          uint32_t subtrees_to_skip,
                          uint32_t *frontier_traversal_idx,
                          uint32_t input_node_relative_depth,
                          struct sequential_scan_result *result,
                          struct circular_stack *not_yet_traversed,
                          struct circular_stack *subtrees_count) {
  if (subtrees_to_skip == 0) {
    result->child_preorder = input_node_idx;
    result->node_relative_depth = input_node_relative_depth;
    result->subtrees_count_map = NULL;
    result->relative_depth_map = NULL;
    return SUCCESS_ECODE;
  }

  SAFE_OP(push_circular_stack(not_yet_traversed, (char *)&subtrees_to_skip));

  struct node_subtree_info nsi;
  nsi.node_index = input_node_idx;
  nsi.node_relative_depth = input_node_relative_depth;
  nsi.subtree_size = 1;
  SAFE_OP(push_circular_stack(subtrees_count, (char *)&nsi));

  uint32_t current_node_index = input_node_idx;
  uint32_t depth = input_node_relative_depth;
  uint32_t real_depth = depth + input_block->block_depth;

  int is_empty;
  SAFE_OP(empty_circular_stack(not_yet_traversed, &is_empty));
  while (!is_empty) {
    current_node_index++;

    int is_frontier;
    CHECK_ERR(frontier_check(input_block->bf, input_node_idx,
                             &frontier_traversal_idx, &is_frontier));

    int reaching_leaf = (real_depth == input_block->tree_depth - 1);

    uint32_t current_not_yet_traversed;
    back_circular_stack(not_yet_traversed, (char *)&current_not_yet_traversed);

    if (!reaching_leaf && !is_frontier) {
      depth++;
    }

    struct node_subtree_info nsi_w;
    nsi_w.node_index = input_node_idx;
    nsi_w.node_relative_depth = depth;
    nsi_w.subtree_size = 1;
    SAFE_OP(push_circular_stack(subtrees_count, (char *)&nsi_w));

    CHECK_ERR(frontier_check(input_block->bf, current_node_index,
                             &frontier_traversal_idx, &is_frontier));
    real_depth = depth + input_block->block_depth;
    reaching_leaf = (real_depth == input_block->tree_depth - 1);

    uint32_t current_children_count;
    count_children(input_block->bt, current_node_index,
                   &current_children_count);

    if (current_children_count > 0 && !reaching_leaf && !is_frontier) {
      SAFE_OP(push_circular_stack(not_yet_traversed,
                                  (char *)&current_children_count));
    } else {
      uint32_t next_nyt;
      SAFE_OP(pop_back_circular_stack(not_yet_traversed, &next_nyt));
      next_nyt--;

      struct node_subtree_info nsi_rep;

      int is_info_stack_empty;
      SAFE_OP(empty_circular_stack(subtrees_count, &is_info_stack_empty));
      if (!is_info_stack_empty) {
        SAFE_OP(pop_back_circular_stack(subtrees_count, (char *)&nsi_rep));
        CHECK_ERR(mark_subtree_size(result, nsi_rep.node_index,
                                    nsi_rep.node_relative_depth,
                                    nsi_rep.subtree_size));
        SAFE_OP(empty_circular_stack(subtrees_count, &is_info_stack_empty));
        if (!is_info_stack_empty) {
          struct node_subtree_info *nsi_rep_within;
          back_reference_circular_stack(subtrees_count,
                                        (char **)&nsi_rep_within);
          nsi_rep_within->subtree_size += nsi_rep.subtree_size;
        }
      }
      SAFE_OP(empty_circular_stack(not_yet_traversed, &is_empty));
      while (next_nyt == 0 && !is_empty) {
        SAFE_OP(pop_back_circular_stack(not_yet_traversed, &next_nyt));
        next_nyt--;

        if (!is_info_stack_empty) {
          SAFE_OP(pop_back_circular_stack(subtrees_count, (char *)&nsi_rep));
          CHECK_ERR(mark_subtree_size(result, nsi_rep.node_index,
                                      nsi_rep.node_relative_depth,
                                      nsi_rep.subtree_size));
          SAFE_OP(empty_circular_stack(subtrees_count, &is_info_stack_empty));
          if (!is_info_stack_empty) {
            struct node_subtree_info *nsi_rep_within;
            SAFE_OP(back_reference_circular_stack(subtrees_count,
                                                  (char **)&nsi_rep_within));
            nsi_rep_within->subtree_size += nsi_rep.subtree_size;
          }
        }

        if (depth > 0) {
          depth--;
        }
        // Last op
        SAFE_OP(empty_circular_stack(not_yet_traversed, &is_empty));
      }

      if (next_nyt > 0) {
        push_circular_stack(not_yet_traversed, (char *)&next_nyt);
      }
    }

    // Last op
    SAFE_OP(empty_circular_stack(not_yet_traversed, &is_empty));
  }
  int is_info_stack_empty;
  SAFE_OP(empty_circular_stack(subtrees_count, &is_info_stack_empty));
  if (!is_info_stack_empty) {
    struct node_subtree_info nsi_out_w;
    SAFE_OP(pop_back_circular_stack(subtrees_count, (char *)&nsi_out_w));
    CHECK_ERR(mark_subtree_size(result, nsi_out_w.node_index,
                                nsi_out_w.node_relative_depth,
                                nsi_out_w.subtree_size));

    SAFE_OP(empty_circular_stack(subtrees_count, &is_info_stack_empty));
    if (!is_info_stack_empty) {
      struct node_subtree_info *nsi_rep_within;
      SAFE_OP(back_reference_circular_stack(subtrees_count,
                                            (char **)&nsi_rep_within));
      nsi_rep_within->subtree_size += nsi_out_w.subtree_size;
    }
  }

  result->child_preorder = current_node_index;
  result->node_relative_depth = depth;

  return SUCCESS_ECODE;
}
/* END INTERNAL FUNCTIONS IMPLEMENTATIONS */

/* EXTERNAL FUNCTIONS */

int has_point(struct block *input_block, uint32_t col, uint32_t row) {
  return 0;
}

int insert_point(struct block *input_block, uint32_t col, uint32_t row) {
  return 0;
}

#pragma clang diagnostic pop

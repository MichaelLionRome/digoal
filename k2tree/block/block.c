#include <math.h>
#include <string.h>

#include <bitvector.h>
#include <circular_queue.h>
#include <vector.h>

#include "block-frontier/block_frontier.h"
#include "block.h"
#include "definitions.h"
#include "morton-code/morton_code.h"

struct child_result {
  struct block *resulting_block;
  uint32_t resulting_node_idx;
  uint32_t resulting_relative_depth;
  int is_leaf_result;
  int exists;

  /* Variables to handle frontier issues */
  int check_frontier; /* True if descended into a frontier node and couldn't
                         find the requested node there */

  int went_frontier; /* True if descended into a frontier node */

  struct block *previous_block;
  uint32_t previous_preorder;
  uint32_t previous_to_current_index;
  uint32_t previous_depth;
};

struct point_search_result {
  struct child_result last_child_result_reached;
  uint32_t depth_reached;
  int point_exists;
  uint32_t treedepth;
};

struct insertion_location {
  uint32_t insertion_index;
  struct point_search_result parent_node;
  uint32_t remaining_depth;
};

struct split_location {
  uint32_t new_frontier_node_position;
  uint32_t new_frontier_node_relative_depth;
};

uint32_t skip_table[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
                         0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 2, 0, 0, 1, 2,
                         0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 2, 0, 1, 1, 2,
                         0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 3, 0, 1, 2, 3};

static inline uint32_t get_subtree_skipping_qty(struct block *b,
                                                uint32_t node_idx,
                                                uint32_t child_idx) {
  uint32_t node_value;
  _SAFE_OP_K2(read_node(b->bt, node_idx, &node_value));
  return skip_table[4 * node_value + child_idx];
}

static inline int mark_subtree_size(struct sequential_scan_result *sc_result,
                                    uint32_t node_index,
                                    uint32_t node_relative_depth,
                                    uint32_t subtree_size) {
  _SAFE_OP_K2(set_element_at(sc_result->subtrees_count_map,
                             (char *)&subtree_size, node_index));
  _SAFE_OP_K2(set_element_at(sc_result->relative_depth_map,
                             (char *)&node_relative_depth, node_index));

  return SUCCESS_ECODE;
}

static inline int block_has_enough_space(struct block *input_block,
                                         struct insertion_location *il) {
  uint32_t allocated_nodes = get_allocated_nodes(input_block->bt);
  return il->remaining_depth <=
         (allocated_nodes - input_block->bt->nodes_count);
}

/* PRIVATE FUNCTIONS PROTOTYPES */
int sequential_scan_child(struct block *input_block, uint32_t input_node_idx,
                          uint32_t subtrees_to_skip,
                          uint32_t *frontier_traversal_idx,
                          uint32_t input_node_relative_depth,
                          struct queries_state *qs);

/**
  Posible return codes:
  SUCCESS_CODE: child was found and stored in output variables
  DOES_NOT_EXIST_CHILD_ERR: child doesn't exist and output variables don't hold
**/
int child(struct block *input_block, uint32_t input_node_idx,
          uint32_t requested_child_position, uint32_t input_node_relative_depth,
          struct child_result *result, struct queries_state *qs);

int find_point(struct block *input_block, struct queries_state *qs,
               struct point_search_result *psr);

int find_insertion_location(struct block *input_block, struct queries_state *qs,
                            struct insertion_location *result);

int get_previous_siblings_count(struct block *input_block,
                                struct child_result *parent_node_result,
                                uint32_t child_code);

int make_room(struct block *input_block, struct insertion_location *il);
int insert_point_mc(struct block *input_block, struct morton_code *mc,
                    struct insertion_location *il);
int make_new_block(struct block *input_block, uint32_t from, uint32_t to,
                   uint32_t relative_depth, struct block **new_block);
int split_block(struct block *input_block, struct queries_state *qs);
int reset_sequential_scan_child(struct queries_state *qs,
                                struct block *target_block);

int insert_point_at(struct block *insertion_block,
                    struct insertion_location *il, struct queries_state *qs);

int reset_vector_to_size(struct vector *v, int size);

/* END PRIVATE FUNCTIONS  PROTOTYPES */

/* PRIVATE FUNCTIONS IMPLEMENTATIONS */

int child(struct block *input_block, uint32_t input_node_idx,
          uint32_t requested_child_position, uint32_t input_node_relative_depth,
          struct child_result *result, struct queries_state *qs) {
  uint32_t tree_depth = input_block->tree_depth;
  if (input_block->block_depth + input_node_relative_depth + 1 == tree_depth) {
    /* Create leaf result */
    result->resulting_block = input_block;
    result->resulting_node_idx = input_node_idx;
    result->resulting_relative_depth = input_node_relative_depth;
    result->is_leaf_result = TRUE;
    return SUCCESS_ECODE;
  }

  uint32_t frontier_traversal_idx = 0;
  int is_frontier;
  CHECK_ERR(frontier_check(input_block->bf, input_node_idx,
                           &frontier_traversal_idx, &is_frontier));

  if (is_frontier) {
    struct block *child_block;
    get_child_block(input_block->bf, frontier_traversal_idx, &child_block);
    int child_err_code =
        child(child_block, 0, requested_child_position, 0, result, qs);
    if (child_err_code != SUCCESS_ECODE &&
        child_err_code != DOES_NOT_EXIST_CHILD_ERR) {
      return child_err_code;
    }

    result->previous_block = input_block;
    result->previous_preorder = input_node_idx;
    result->previous_to_current_index = requested_child_position;
    result->previous_depth = input_node_relative_depth;
    if (result->resulting_relative_depth +
            result->resulting_block->block_depth + 1 ==
        result->resulting_block->tree_depth) {
      result->is_leaf_result = TRUE;
    }
    result->check_frontier = !result->exists;
    result->went_frontier = TRUE;

    return child_err_code;
  }

  int exists;
  CHECK_ERR(child_exists(input_block->bt, input_node_idx,
                         requested_child_position, &exists));
  if (!exists) {
    result->exists = FALSE;
    result->resulting_block = input_block;
    return DOES_NOT_EXIST_CHILD_ERR;
  }

  uint32_t subtrees_to_skip = get_subtree_skipping_qty(
      input_block, input_node_idx, requested_child_position);
  qs->find_split_data = FALSE;
  CHECK_ERR(sequential_scan_child(input_block, input_node_idx, subtrees_to_skip,
                                  &frontier_traversal_idx,
                                  input_node_relative_depth, qs));

  result->resulting_block = input_block;
  result->resulting_node_idx = qs->sc_result.child_preorder + 1;
  result->resulting_relative_depth = input_node_relative_depth + 1;
  result->exists = TRUE;

  return SUCCESS_ECODE;
}

int reset_sequential_scan_child(struct queries_state *qs,
                                struct block *target_block) {
  reset_circular_queue(&qs->not_yet_traversed);
  qs->sc_result.child_preorder = 0;
  qs->sc_result.node_relative_depth = 0;

  if (qs->find_split_data) {
    reset_circular_queue(&qs->subtrees_count);
    CHECK_ERR(reset_vector_to_size(qs->sc_result.subtrees_count_map,
                                   target_block->bt->nodes_count));
    CHECK_ERR(reset_vector_to_size(qs->sc_result.relative_depth_map,
                                   target_block->bt->nodes_count));
  }

  return SUCCESS_ECODE;
}

int sequential_scan_child(struct block *input_block, uint32_t input_node_idx,
                          uint32_t subtrees_to_skip,
                          uint32_t *frontier_traversal_idx,
                          uint32_t input_node_relative_depth,
                          struct queries_state *qs) {

  reset_sequential_scan_child(qs, input_block);

  struct sequential_scan_result *result = &qs->sc_result;

  if (subtrees_to_skip == 0) {
    result->child_preorder = input_node_idx;
    result->node_relative_depth = input_node_relative_depth;
    return SUCCESS_ECODE;
  }

  _SAFE_OP_K2(
      push_circular_queue(&qs->not_yet_traversed, (char *)&subtrees_to_skip));

  if (qs->find_split_data) {
    struct node_subtree_info nsi;
    nsi.node_index = input_node_idx;
    nsi.node_relative_depth = input_node_relative_depth;
    nsi.subtree_size = 1;
    _SAFE_OP_K2(push_circular_queue(&qs->subtrees_count, (char *)&nsi));
  }

  uint32_t current_node_index = input_node_idx;
  uint32_t depth = input_node_relative_depth;
  uint32_t real_depth = depth + input_block->block_depth;

  int is_empty;
  _SAFE_OP_K2(empty_circular_queue(&qs->not_yet_traversed, &is_empty));
  while (!is_empty) {
    current_node_index++;

    int is_frontier;
    CHECK_ERR(frontier_check(input_block->bf, input_node_idx,
                             frontier_traversal_idx, &is_frontier));

    int reaching_leaf = (real_depth == input_block->tree_depth - 1);

    uint32_t current_not_yet_traversed;
    _SAFE_OP_K2(back_circular_queue(&qs->not_yet_traversed,
                                    (char *)&current_not_yet_traversed));

    if (!reaching_leaf && !is_frontier) {
      depth++;
    }

    /* start block of split data */
    if (qs->find_split_data) {
      struct node_subtree_info nsi_w;
      nsi_w.node_index = current_node_index;
      nsi_w.node_relative_depth = depth;
      nsi_w.subtree_size = 1;
      _SAFE_OP_K2(push_circular_queue(&qs->subtrees_count, (char *)&nsi_w));
    }
    /* end block of split data */

    CHECK_ERR(frontier_check(input_block->bf, current_node_index,
                             frontier_traversal_idx, &is_frontier));
    real_depth = depth + input_block->block_depth;
    reaching_leaf = (real_depth == input_block->tree_depth - 1);

    uint32_t current_children_count;
    count_children(input_block->bt, current_node_index,
                   &current_children_count);

    if (current_children_count > 0 && !reaching_leaf && !is_frontier) {
      _SAFE_OP_K2(push_circular_queue(&qs->not_yet_traversed,
                                      (char *)&current_children_count));
    } else {
      uint32_t next_nyt;
      _SAFE_OP_K2(
          pop_back_circular_queue(&qs->not_yet_traversed, (char *)&next_nyt));
      next_nyt--;

      int is_info_queue_empty = TRUE;
      struct node_subtree_info nsi_rep;
      /* start block of split data */
      if (qs->find_split_data) {
        _SAFE_OP_K2(
            empty_circular_queue(&qs->subtrees_count, &is_info_queue_empty));
        if (!is_info_queue_empty) {
          _SAFE_OP_K2(
              pop_back_circular_queue(&qs->subtrees_count, (char *)&nsi_rep));
          CHECK_ERR(mark_subtree_size(result, nsi_rep.node_index,
                                      nsi_rep.node_relative_depth,
                                      nsi_rep.subtree_size));
          _SAFE_OP_K2(
              empty_circular_queue(&qs->subtrees_count, &is_info_queue_empty));
          if (!is_info_queue_empty) {
            struct node_subtree_info *nsi_rep_within;
            back_reference_circular_queue(&qs->subtrees_count,
                                          (char **)&nsi_rep_within);
            nsi_rep_within->subtree_size += nsi_rep.subtree_size;
          }
        }
      }
      /* end block of split data */

      _SAFE_OP_K2(empty_circular_queue(&qs->not_yet_traversed, &is_empty));
      while (next_nyt == 0 && !is_empty) {
        _SAFE_OP_K2(
            pop_back_circular_queue(&qs->not_yet_traversed, (char *)&next_nyt));
        next_nyt--;

        /* start block of split data */
        if (qs->find_split_data && !is_info_queue_empty) {
          _SAFE_OP_K2(
              pop_back_circular_queue(&qs->subtrees_count, (char *)&nsi_rep));
          CHECK_ERR(mark_subtree_size(result, nsi_rep.node_index,
                                      nsi_rep.node_relative_depth,
                                      nsi_rep.subtree_size));
          _SAFE_OP_K2(
              empty_circular_queue(&qs->subtrees_count, &is_info_queue_empty));
          if (!is_info_queue_empty) {
            struct node_subtree_info *nsi_rep_within;
            _SAFE_OP_K2(back_reference_circular_queue(
                &qs->subtrees_count, (char **)&nsi_rep_within));
            nsi_rep_within->subtree_size += nsi_rep.subtree_size;
          }
        }
        /* end block of split data */

        if (depth > 0) {
          depth--;
        }
        // Last op
        _SAFE_OP_K2(empty_circular_queue(&qs->not_yet_traversed, &is_empty));
      }
      if (is_frontier && depth > 0) {
        depth--;
      }
      if (next_nyt > 0) {
        _SAFE_OP_K2(
            push_circular_queue(&qs->not_yet_traversed, (char *)&next_nyt));
      }
    }

    // Last op
    _SAFE_OP_K2(empty_circular_queue(&qs->not_yet_traversed, &is_empty));
  }
  /* end block of split data */
  if (qs->find_split_data) {
    int is_info_queue_empty;
    _SAFE_OP_K2(
        empty_circular_queue(&qs->subtrees_count, &is_info_queue_empty));
    if (!is_info_queue_empty) {
      struct node_subtree_info nsi_out_w;
      _SAFE_OP_K2(
          pop_back_circular_queue(&qs->subtrees_count, (char *)&nsi_out_w));
      CHECK_ERR(mark_subtree_size(result, nsi_out_w.node_index,
                                  nsi_out_w.node_relative_depth,
                                  nsi_out_w.subtree_size));

      _SAFE_OP_K2(
          empty_circular_queue(&qs->subtrees_count, &is_info_queue_empty));
      if (!is_info_queue_empty) {
        struct node_subtree_info *nsi_rep_within;
        _SAFE_OP_K2(back_reference_circular_queue(&qs->subtrees_count,
                                                  (char **)&nsi_rep_within));
        nsi_rep_within->subtree_size += nsi_out_w.subtree_size;
      }
    }
  }
  /* end block of split data */

  result->child_preorder = current_node_index;
  result->node_relative_depth = depth;

  return SUCCESS_ECODE;
}

int find_point(struct block *input_block, struct queries_state *qs,
               struct point_search_result *psr) {
  struct child_result current_cr;
  current_cr.resulting_node_idx = 0;
  current_cr.resulting_block = input_block;
  current_cr.resulting_relative_depth = 0;
  current_cr.is_leaf_result = FALSE;
  current_cr.previous_block = NULL;
  current_cr.check_frontier = FALSE;
  current_cr.went_frontier = FALSE;

  uint32_t depth = input_block->block_depth;
  uint32_t relative_depth = 0;
  for (depth = input_block->block_depth; depth < input_block->tree_depth;
       depth++) {
    relative_depth = depth - current_cr.resulting_block->block_depth;
    struct child_result prev_cr = current_cr;
    uint32_t current_mcode;
    CHECK_ERR(get_code_at_morton_code(&qs->mc, depth, &current_mcode));

    int child_err_code =
        child(current_cr.resulting_block, current_cr.resulting_node_idx,
              current_mcode, relative_depth, &current_cr, qs);
    if (child_err_code != 0 && child_err_code != DOES_NOT_EXIST_CHILD_ERR) {
      return child_err_code;
    }

    if (current_cr.check_frontier && !current_cr.exists &&
        current_cr.resulting_block != NULL &&
        current_cr.resulting_block != input_block) {
      prev_cr.previous_block = prev_cr.resulting_block;
      prev_cr.resulting_block = current_cr.resulting_block;

      prev_cr.previous_depth = prev_cr.resulting_relative_depth;
      prev_cr.previous_preorder = prev_cr.resulting_node_idx;
      prev_cr.previous_to_current_index = current_mcode;

      prev_cr.resulting_node_idx = 0;
      prev_cr.resulting_relative_depth = 0;
    }

    psr->treedepth = input_block->tree_depth;

    if (!current_cr.exists) {
      psr->last_child_result_reached = prev_cr;
      psr->depth_reached = depth;
      psr->point_exists = FALSE;
      return SUCCESS_ECODE;
    }

    if (current_cr.is_leaf_result) {
      uint32_t leaf_code;
      _SAFE_OP_K2(leaf_child_morton_code(&qs->mc, &leaf_code));

      int does_child_exists;
      _SAFE_OP_K2(child_exists(current_cr.resulting_block->bt,
                               current_cr.resulting_node_idx, leaf_code,
                               &does_child_exists));
      psr->last_child_result_reached = current_cr;
      psr->depth_reached = depth + 1;
      psr->point_exists = does_child_exists;
      return SUCCESS_ECODE;
    }
  }

  // If the loop ends, then we have found a point
  psr->last_child_result_reached = current_cr;
  psr->depth_reached = depth - 1;
  psr->point_exists = TRUE;

  return SUCCESS_ECODE;
}

int find_insertion_location(struct block *input_block, struct queries_state *qs,
                            struct insertion_location *result) {
  struct point_search_result psr;
  CHECK_ERR(find_point(input_block, qs, &psr));

  struct block *reached_block = psr.last_child_result_reached.resulting_block;

  uint32_t frontier_traversal_idx = 0;
  uint32_t node_index = psr.last_child_result_reached.resulting_node_idx;

  result->parent_node = psr;

  if (psr.point_exists || psr.depth_reached == input_block->tree_depth - 1 ||
      psr.last_child_result_reached.is_leaf_result) {
    result->insertion_index = node_index;
    result->remaining_depth = 0;
    return SUCCESS_ECODE;
  }

  int is_frontier;
  CHECK_ERR(frontier_check(reached_block->bf, node_index,
                           &frontier_traversal_idx, &is_frontier));
  if (is_frontier) {
    return FRONTIER_NODE_WITHIN_FIND_INSERTION_LOC;
  }

  uint32_t child_code;
  CHECK_ERR(get_code_at_morton_code(&qs->mc, psr.depth_reached, &child_code));

  uint32_t to_be_skipped_subtrees = get_previous_siblings_count(
      reached_block, &psr.last_child_result_reached, child_code);

  if (to_be_skipped_subtrees == 0 && psr.depth_reached == 0) {
    if (reached_block->bt->nodes_count == 0) {
      result->insertion_index = 0;
      result->remaining_depth = input_block->tree_depth - psr.depth_reached;
    } else {
      result->insertion_index = 1;
      result->remaining_depth = input_block->tree_depth - psr.depth_reached - 1;
    }
    return SUCCESS_ECODE;
  }

  qs->find_split_data = FALSE;
  CHECK_ERR(sequential_scan_child(
      reached_block, node_index, to_be_skipped_subtrees,
      &frontier_traversal_idx, psr.depth_reached - reached_block->block_depth,
      qs));

  result->insertion_index = qs->sc_result.child_preorder + 1;
  result->remaining_depth = input_block->tree_depth - 1 - psr.depth_reached;

  return SUCCESS_ECODE;
}

int get_previous_siblings_count(struct block *input_block,
                                struct child_result *parent_node_result,
                                uint32_t child_code) {
  if (parent_node_result->resulting_node_idx >= input_block->bt->nodes_count) {
    return 0;
  }
  return get_subtree_skipping_qty(
      input_block, parent_node_result->resulting_node_idx, child_code);
}

int make_room(struct block *input_block, struct insertion_location *il) {
  uint32_t next_node_index = il->insertion_index;
  uint32_t nodes_to_insert = il->remaining_depth;
  uint32_t occupied_nodes = input_block->bt->nodes_count;

  if (nodes_to_insert == 0) {
    return SUCCESS_ECODE;
  }

  if (occupied_nodes < next_node_index + nodes_to_insert) {
    CHECK_ERR(enlarge_block_size_to(input_block->bt,
                                    occupied_nodes + nodes_to_insert));
  } else {
    CHECK_ERR(shift_right_nodes_after(input_block->bt, next_node_index - 1,
                                      nodes_to_insert));
  }

  return SUCCESS_ECODE;
}

int insert_point_mc(struct block *input_block, struct morton_code *mc,
                    struct insertion_location *il) {
  uint32_t current_index;
  if (il->remaining_depth == input_block->tree_depth - 1 &&
      il->insertion_index == 0) {
    current_index = 0;
  } else {
    current_index = il->insertion_index;
  }

  for (uint32_t code_idx = input_block->tree_depth - il->remaining_depth;
       code_idx < input_block->tree_depth; code_idx++) {
    uint32_t code;
    CHECK_ERR(get_code_at_morton_code(mc, code_idx, &code));
    CHECK_ERR(insert_node_at(input_block->bt, current_index++, code));
  }

  CHECK_ERR(fix_frontier_indexes(input_block->bf, il->insertion_index,
                                 -((int)il->remaining_depth)));

  return SUCCESS_ECODE;
}

int make_new_block(struct block *input_block, uint32_t from, uint32_t to,
                   uint32_t relative_depth, struct block **new_block) {
  struct block *created_block = calloc(1, sizeof(struct block));

  /* initialize block topology */
  struct block_topology *bt = calloc(1, sizeof(struct block_topology));
  struct bitvector *bv = calloc(1, sizeof(struct bitvector));
  uint32_t new_bv_start_pos = 4 * from;
  uint32_t new_bv_end_pos = 4 * (to + 1) - 1;
  uint32_t new_bv_size = new_bv_end_pos - new_bv_start_pos + 1;
  _SAFE_OP_K2(init_bitvector(bv, new_bv_size));
  CHECK_ERR(extract_sub_bitvector(input_block->bt, new_bv_start_pos,
                                  new_bv_end_pos, bv));
  /* shrink parent bitvector */
  CHECK_ERR(collapse_nodes(input_block->bt, from + 1, to));
  CHECK_ERR(init_block_topology(bt, bv, to - from + 1));
  /* initialize block frontier */
  struct block_frontier *bf = calloc(1, sizeof(struct block_frontier));
  CHECK_ERR(extract_sub_block_frontier(input_block->bf, from, to, bf));
  // CHECK_ERR(init_block_frontier_with_capacity(bf, ));

  created_block->bt = bt;
  created_block->bf = bf;
  created_block->block_depth = relative_depth;
  created_block->tree_depth = input_block->tree_depth;
  created_block->max_node_count = input_block->max_node_count;

  *new_block = created_block;

  return SUCCESS_ECODE;
}

int split_block(struct block *input_block, struct queries_state *qs) {
  /* find split location */
  uint32_t new_frontier_node_position;
  uint32_t new_frontier_node_relative_depth;

  uint32_t traversal_frontier_idx = 0;
  uint32_t children_count;

  CHECK_ERR(count_children(input_block->bt, 0, &children_count));
  qs->find_split_data = TRUE;
  CHECK_ERR(sequential_scan_child(input_block, 0, children_count,
                                  &traversal_frontier_idx, 0, qs));
  qs->find_split_data = FALSE;

  int leftmost_not_frontier = -1;
  int leftmost_not_frontier_depth = -1;
  int is_frontier = FALSE;
  uint32_t frontier_traversal_index = 0;

  int found_loc = FALSE;
  for (uint32_t node_index = 1; node_index <= qs->sc_result.child_preorder;
       node_index++) {
    CHECK_ERR(frontier_check(input_block->bf, node_index,
                             &frontier_traversal_index, &is_frontier));
    if (is_frontier) {
      continue;
    } else {
      if (leftmost_not_frontier == -1) {
        leftmost_not_frontier = node_index;
        leftmost_not_frontier_depth =
            read_uint_element(qs->sc_result.relative_depth_map, node_index);
      }
    }
    uint32_t subtree_size =
        read_uint_element(qs->sc_result.subtrees_count_map, node_index);
    if (subtree_size >= input_block->bt->nodes_count / 4) {
      new_frontier_node_position = node_index;
      new_frontier_node_relative_depth =
          read_uint_element(qs->sc_result.relative_depth_map, node_index);
      found_loc = TRUE;
      break;
    }
  }

  if (!found_loc) {
    new_frontier_node_position = 1;
    new_frontier_node_relative_depth = 1;
    if (leftmost_not_frontier != -1) {
      new_frontier_node_position = (uint32_t)leftmost_not_frontier;
      new_frontier_node_relative_depth = (uint32_t)leftmost_not_frontier_depth;
    }
  }

  /* split block */
  traversal_frontier_idx = 0;
  CHECK_ERR(count_children(input_block->bt, new_frontier_node_position,
                           &children_count));
  qs->find_split_data = FALSE;
  CHECK_ERR(sequential_scan_child(input_block, new_frontier_node_position,
                                  children_count, &traversal_frontier_idx,
                                  new_frontier_node_relative_depth, qs));

  uint32_t right_index = qs->sc_result.child_preorder;

  struct block *new_block;
  CHECK_ERR(make_new_block(
      input_block, new_frontier_node_position, right_index,
      new_frontier_node_relative_depth + input_block->block_depth, &new_block));

  new_block->root = input_block->root;

  CHECK_ERR(
      fix_frontier_indexes(new_block->bf, 0, new_frontier_node_position + 1));

  int delta_indexes_parent = right_index - new_frontier_node_position;

  /*
    CHECK_ERR(collapse_frontier_nodes(input_block->bf,
    new_frontier_node_position + 1, right_index));*/
  CHECK_ERR(fix_frontier_indexes(
      input_block->bf, new_frontier_node_position + 1, delta_indexes_parent));

  CHECK_ERR(add_frontier_node(input_block->bf, new_frontier_node_position,
                              new_block));

  return SUCCESS_ECODE;
}

int insert_point_at(struct block *insertion_block,
                    struct insertion_location *il, struct queries_state *qs) {

  if (block_has_enough_space(insertion_block, il)) {
    CHECK_ERR(make_room(insertion_block, il));
    struct child_result *lcresult =
        &(il->parent_node).last_child_result_reached;
    int child_node_is_parent =
        il->insertion_index == lcresult->resulting_node_idx;
    if (!child_node_is_parent) {
      uint32_t node_code;
      CHECK_ERR(get_code_at_morton_code(&qs->mc, il->parent_node.depth_reached,
                                        &node_code));
      CHECK_ERR(mark_child_in_node(
          insertion_block->bt,
          il->parent_node.last_child_result_reached.resulting_node_idx,
          node_code));

      struct block *previous_block = lcresult->previous_block;
      if (lcresult->went_frontier) {
        CHECK_ERR(mark_child_in_node(previous_block->bt,
                                     lcresult->previous_preorder,
                                     lcresult->previous_to_current_index));
      }
    }

    if (il->remaining_depth == 0) {
      uint32_t leaf_child;
      CHECK_ERR(leaf_child_morton_code(&qs->mc, &leaf_child));
      CHECK_ERR(mark_child_in_node(insertion_block->bt, il->insertion_index,
                                   leaf_child));
    } else {
      CHECK_ERR(insert_point_mc(insertion_block, &qs->mc, il));
    }

    return SUCCESS_ECODE;
  }

  // Check if can enlarge block to fit
  uint32_t next_amount_of_nodes =
      insertion_block->bt->nodes_count + il->remaining_depth;
  if (next_amount_of_nodes <= insertion_block->max_node_count) {
    uint32_t next_block_sz = 1 << (uint32_t)ceil(log2(next_amount_of_nodes));
    CHECK_ERR(enlarge_block_size_to(insertion_block->bt, next_block_sz));
    return insert_point_at(insertion_block, il, qs);
  }

  CHECK_ERR(split_block(insertion_block, qs));

  struct insertion_location il_split;
  CHECK_ERR(find_insertion_location(insertion_block->root, qs, &il_split));

  return insert_point_at(
      il_split.parent_node.last_child_result_reached.resulting_block, &il_split,
      qs);
}

int reset_vector_to_size(struct vector *v, int size) {
  if (size > v->capacity) {
    return RESET_SIZE_HIGHER_THAN_CAPACITY;
  }

  int bytes_to_reset = size * v->element_size;
  memset(v->data, 0, bytes_to_reset);
  v->nof_items = size;

  return SUCCESS_ECODE;
}

/* END PRIVATE FUNCTIONS IMPLEMENTATIONS */

/* PUBLIC FUNCTIONS */

int has_point(struct block *input_block, ulong col, ulong row,
              struct queries_state *qs, int *result) {

  CHECK_ERR(convert_coordinates_to_morton_code(
      col, row, input_block->tree_depth, &qs->mc));

  struct point_search_result psr;
  CHECK_ERR(find_point(input_block, qs, &psr));

  *result = psr.point_exists;

  return SUCCESS_ECODE;
}

int insert_point(struct block *input_block, ulong col, ulong row,
                 struct queries_state *qs) {
  CHECK_ERR(convert_coordinates_to_morton_code(
      col, row, input_block->tree_depth, &qs->mc));
  struct insertion_location il;
  CHECK_ERR(find_insertion_location(input_block, qs, &il));
  struct block *insertion_block =
      il.parent_node.last_child_result_reached.resulting_block;
  return insert_point_at(insertion_block, &il, qs);
}

struct block *create_block(uint32_t tree_depth) {
  struct block *new_block = calloc(1, sizeof(struct block));
  new_block->bt = create_block_topology();
  new_block->bf = create_block_frontier();
  new_block->block_depth = 0;
  new_block->tree_depth = tree_depth;
  new_block->max_node_count = MAX_NODES_IN_BLOCK;
  new_block->root = new_block;
  return new_block;
}

int free_rec_block(struct block *input_block) {
  struct vector *frontier_blocks = &input_block->bf->blocks;
  if (frontier_blocks->nof_items == 0) {
    return free_block(input_block);
  }

  for (int i = 0; i < frontier_blocks->nof_items; i++) {
    struct block *current_block = read_block_element(frontier_blocks, i);
    CHECK_ERR(free_rec_block(current_block));
  }

  return free_block(input_block);
}

int free_block(struct block *input_block) {
  CHECK_ERR(free_block_topology(input_block->bt));
  CHECK_ERR(free_block_frontier(input_block->bf));
  free(input_block->bt);
  free(input_block->bf);
  free(input_block);
  return SUCCESS_ECODE;
}

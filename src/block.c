/*
MIT License

Copyright (c) 2020 Cristobal Miranda T.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <math.h>
#include <string.h>

#include "block.h"
#include "block_frontier.h"
#include "definitions.h"
#include "morton_code.h"
#include "stacks.h"
#include "vectors.h"
#include <assert.h>
#include <bitvector.h>

#include "memalloc.h"

#ifdef DEBUG_STATS
#include <sys/time.h>
#endif

struct point_search_result {
  struct child_result last_child_result_reached;
  TREE_DEPTH_T depth_reached;
  int point_exists;
  TREE_DEPTH_T treedepth;
};

struct insertion_location {
  uint32_t insertion_index;
  struct point_search_result parent_node;
  TREE_DEPTH_T remaining_depth;
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
  _SAFE_OP_K2(read_node(b, node_idx, &node_value));
  return skip_table[4 * node_value + child_idx];
}

static inline int mark_subtree_size(struct sequential_scan_result *sc_result,
                                    uint32_t node_index,
                                    TREE_DEPTH_T node_relative_depth,
                                    uint32_t subtree_size) {

  sc_result->subtrees_count_map[node_index] = subtree_size;
  sc_result->relative_depth_map[node_index] = node_relative_depth;

  return SUCCESS_ECODE_K2T;
}

static inline int block_has_enough_space(struct block *input_block,
                                         struct insertion_location *il) {
  uint32_t allocated_nodes = get_allocated_nodes(input_block);
  return il->remaining_depth <=
         (allocated_nodes - get_nodes_count(input_block));
}

int clean_child_result(struct child_result *cresult) {
  cresult->resulting_block = NULL;
  cresult->block_depth = 0;
  cresult->resulting_node_idx = 0;
  cresult->resulting_relative_depth = 0;
  cresult->is_leaf_result = FALSE;
  cresult->exists = FALSE;
  cresult->check_frontier = FALSE;
  cresult->went_frontier = FALSE;
  cresult->previous_block = NULL;
  cresult->previous_preorder = 0;
  cresult->previous_to_current_index = 0;
  cresult->previous_depth = 0;
  return SUCCESS_ECODE_K2T;
}

/* PRIVATE FUNCTIONS PROTOTYPES */
int sequential_scan_child(struct block *input_block, uint32_t input_node_idx,
                          uint32_t subtrees_to_skip,
                          uint32_t *frontier_traversal_idx,
                          TREE_DEPTH_T input_node_relative_depth,
                          struct queries_state *qs, TREE_DEPTH_T block_depth);

/**
  Posible return codes:
  SUCCESS_CODE: child was found and stored in output variables
  DOES_NOT_EXIST_CHILD_ERR: child doesn't exist and output variables don't hold
**/
int child(struct block *input_block, uint32_t input_node_idx,
          uint32_t requested_child_position,
          TREE_DEPTH_T input_node_relative_depth, struct child_result *result,
          struct queries_state *qs, TREE_DEPTH_T block_depth);

int find_point(struct block *input_block, struct queries_state *qs,
               struct point_search_result *psr, TREE_DEPTH_T block_depth);

int find_insertion_location(struct block *input_block, struct queries_state *qs,
                            struct insertion_location *result,
                            TREE_DEPTH_T block_depth);

int get_previous_siblings_count(struct block *input_block,
                                struct child_result *parent_node_result,
                                uint32_t child_code);

int make_room(struct block *input_block, struct insertion_location *il);
int insert_point_mc(struct block *input_block, struct morton_code *mc,
                    struct insertion_location *il);
int make_new_block(struct block *input_block, uint32_t from, uint32_t to,
                   struct block *new_block);
int split_block(struct block *input_block, struct queries_state *qs,
                TREE_DEPTH_T block_depth);
int reset_sequential_scan_child(struct queries_state *qs);

int insert_point_at(struct block *insertion_block,
                    struct insertion_location *il, struct queries_state *qs,
                    TREE_DEPTH_T block_depth, int *already_existed);

int naive_scan_points_rec(struct block *input_block, struct queries_state *qs,
                          struct vector_pair2dl_t *result,
                          struct child_result *cresult,
                          TREE_DEPTH_T block_depth);

int naive_scan_points_rec_interactively(struct block *input_block,
                                        struct queries_state *qs,
                                        point_reporter_fun_t point_reporter,
                                        void *report_state,
                                        struct child_result *cresult,
                                        TREE_DEPTH_T block_depth);

int report_rec(unsigned long current_col, struct queries_state *qs,
               struct vector_pair2dl_t *result, struct child_result *current_cr,
               int which_report);

int report_rec_interactively(unsigned long current_col,
                             struct queries_state *qs,
                             point_reporter_fun_t point_reporter,
                             struct child_result *current_cr, int which_report,
                             void *report_state);

int free_rec_block_internal(struct block *input_block);

int delete_point_rec(struct block *input_block, struct deletion_state *ds,
                     struct child_result cr, int *already_not_exists,
                     int *has_children);

int delete_nodes_in_block(struct block *input_block, struct deletion_state *ds,
                          int *total_deleted);

int merge_blocks(struct block *parent_block, struct block *child_block,
                 uint32_t frontier_node_in_parent);

/* END PRIVATE FUNCTIONS  PROTOTYPES */

/* PRIVATE FUNCTIONS IMPLEMENTATIONS */

int child(struct block *input_block, uint32_t input_node_idx,
          uint32_t requested_child_position,
          TREE_DEPTH_T input_node_relative_depth, struct child_result *result,
          struct queries_state *qs, TREE_DEPTH_T block_depth) {
  TREE_DEPTH_T tree_depth = qs->treedepth;
  if (block_depth + input_node_relative_depth + 1 == tree_depth) {
    /* Create leaf result */
    result->resulting_block = input_block;
    result->resulting_node_idx = input_node_idx;
    result->resulting_relative_depth = input_node_relative_depth;
    result->is_leaf_result = TRUE;
    result->block_depth = block_depth;
    return SUCCESS_ECODE_K2T;
  }

  uint32_t frontier_traversal_idx = 0;
  int is_frontier;
#ifdef DEBUG_STATS
  struct timeval tval_before, tval_after, tval_result;
  gettimeofday(&tval_before, NULL);
#endif
  is_frontier =
      frontier_check(input_block, input_node_idx, &frontier_traversal_idx);

#ifdef DEBUG_STATS
  gettimeofday(&tval_after, NULL);
  timersub(&tval_after, &tval_before, &tval_result);
  qs->dstats.time_on_sequential_scan +=
      (unsigned long)tval_result.tv_sec * 1000000 + tval_result.tv_usec;
#endif

  if (is_frontier) {
    struct block *child_block =
        get_child_block(input_block, frontier_traversal_idx);
    int child_err_code =
        child(child_block, 0, requested_child_position, 0, result, qs,
              block_depth + input_node_relative_depth);
    if (child_err_code != SUCCESS_ECODE_K2T &&
        child_err_code != DOES_NOT_EXIST_CHILD_ERR) {
      return child_err_code;
    }

    result->previous_block = input_block;
    result->previous_preorder = input_node_idx;
    result->previous_to_current_index = requested_child_position;
    result->previous_depth = input_node_relative_depth;
    if (result->resulting_relative_depth + result->block_depth + 1 ==
        qs->treedepth) {
      result->is_leaf_result = TRUE;
    }
    result->check_frontier = !result->exists;
    result->went_frontier = TRUE;

    return child_err_code;
  }

  int exists = 0;
  CHECK_ERR(child_exists(input_block, input_node_idx, requested_child_position,
                         &exists));
  if (!exists) {
    result->exists = FALSE;
    result->resulting_block = input_block;
    result->block_depth = block_depth;
    return DOES_NOT_EXIST_CHILD_ERR;
  }

  uint32_t subtrees_to_skip = get_subtree_skipping_qty(
      input_block, input_node_idx, requested_child_position);
  qs->find_split_data = FALSE;
#ifdef DEBUG_STATS
  gettimeofday(&tval_before, NULL);
#endif
  CHECK_ERR(sequential_scan_child(input_block, input_node_idx, subtrees_to_skip,
                                  &frontier_traversal_idx,
                                  input_node_relative_depth, qs, block_depth));

#ifdef DEBUG_STATS
  gettimeofday(&tval_after, NULL);
  timersub(&tval_after, &tval_before, &tval_result);
  qs->dstats.time_on_sequential_scan +=
      (unsigned long)tval_result.tv_sec * 1000000 + tval_result.tv_usec;
#endif

  result->resulting_block = input_block;
  result->resulting_node_idx = qs->sc_result.child_preorder + 1;
  result->resulting_relative_depth = input_node_relative_depth + 1;
  result->exists = TRUE;
  result->block_depth = block_depth;

  return SUCCESS_ECODE_K2T;
}

int reset_sequential_scan_child(struct queries_state *qs) {
  reset_int_stack(&qs->not_yet_traversed);
  qs->sc_result.child_preorder = 0;
  qs->sc_result.node_relative_depth = 0;

  if (qs->find_split_data) {
    reset_nsi_t_stack(&qs->subtrees_count);
  }

  return SUCCESS_ECODE_K2T;
}

int sequential_scan_child(struct block *input_block, uint32_t input_node_idx,
                          uint32_t subtrees_to_skip,
                          uint32_t *frontier_traversal_idx,
                          TREE_DEPTH_T input_node_relative_depth,
                          struct queries_state *qs, TREE_DEPTH_T block_depth) {

  reset_sequential_scan_child(qs);

  struct sequential_scan_result *result = &qs->sc_result;

  if (subtrees_to_skip == 0) {
    result->child_preorder = input_node_idx;
    result->node_relative_depth = input_node_relative_depth;
    return SUCCESS_ECODE_K2T;
  }

  push_int_stack(&qs->not_yet_traversed, (int)subtrees_to_skip);

  if (qs->find_split_data) {
    struct node_subtree_info nsi;
    nsi.node_index = input_node_idx;
    nsi.node_relative_depth = input_node_relative_depth;
    nsi.subtree_size = 1;
    push_nsi_t_stack(&qs->subtrees_count, nsi);
  }

  uint32_t current_node_index = input_node_idx;
  TREE_DEPTH_T depth = input_node_relative_depth;
  TREE_DEPTH_T real_depth = depth + block_depth;

  while (!empty_int_stack(&qs->not_yet_traversed)) {
    current_node_index++;

    int is_frontier;

#ifdef DEBUG_STATS
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
#endif
    is_frontier =
        frontier_check(input_block, input_node_idx, frontier_traversal_idx);
#ifdef DEBUG_STATS
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    qs->dstats.time_on_frontier_check +=
        (unsigned long)tval_result.tv_sec * 1000000 + tval_result.tv_usec;
#endif

    int reaching_leaf = (real_depth == qs->treedepth - 1);

    if (!reaching_leaf && !is_frontier) {
      depth++;
    }

    /* start block of split data */
    if (qs->find_split_data) {
      struct node_subtree_info nsi_w;
      nsi_w.node_index = current_node_index;
      nsi_w.node_relative_depth = depth;
      nsi_w.subtree_size = 1;
      push_nsi_t_stack(&qs->subtrees_count, nsi_w);
    }
    /* end block of split data */

#ifdef DEBUG_STATS
    gettimeofday(&tval_before, NULL);
#endif
    is_frontier =
        frontier_check(input_block, current_node_index, frontier_traversal_idx);
#ifdef DEBUG_STATS
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    qs->dstats.time_on_frontier_check +=
        (unsigned long)tval_result.tv_sec * 1000000 + tval_result.tv_usec;
#endif
    real_depth = depth + block_depth;
    reaching_leaf = (real_depth == qs->treedepth - 1);

    uint32_t current_children_count;
    count_children(input_block, current_node_index, &current_children_count);

    if (current_children_count > 0 && !reaching_leaf && !is_frontier) {

      push_int_stack(&qs->not_yet_traversed, (int)current_children_count);
    } else {
      int next_nyt = pop_int_stack(&qs->not_yet_traversed) - 1;

      int is_info_queue_empty = TRUE;
      struct node_subtree_info nsi_rep;
      /* start block of split data */
      if (qs->find_split_data) {
        if (!(is_info_queue_empty = empty_nsi_t_stack(&qs->subtrees_count))) {
          nsi_rep = pop_nsi_t_stack(&qs->subtrees_count);
          CHECK_ERR(mark_subtree_size(result, nsi_rep.node_index,
                                      nsi_rep.node_relative_depth,
                                      nsi_rep.subtree_size));
          if (!(is_info_queue_empty = empty_nsi_t_stack(&qs->subtrees_count))) {
            struct node_subtree_info *nsi_rep_within =
                top_ref_nsi_t_stack(&qs->subtrees_count);
            nsi_rep_within->subtree_size += nsi_rep.subtree_size;
          }
        }
      }
      /* end block of split data */
      while (next_nyt == 0 && !empty_int_stack(&qs->not_yet_traversed)) {
        next_nyt = pop_int_stack(&qs->not_yet_traversed) - 1;

        /* start block of split data */
        if (qs->find_split_data && !is_info_queue_empty) {
          nsi_rep = pop_nsi_t_stack(&qs->subtrees_count);
          CHECK_ERR(mark_subtree_size(result, nsi_rep.node_index,
                                      nsi_rep.node_relative_depth,
                                      nsi_rep.subtree_size));

          if (!(is_info_queue_empty = empty_nsi_t_stack(&qs->subtrees_count))) {
            struct node_subtree_info *nsi_rep_within =
                top_ref_nsi_t_stack(&qs->subtrees_count);
            nsi_rep_within->subtree_size += nsi_rep.subtree_size;
          }
        }
        /* end block of split data */

        if (depth > 0) {
          depth--;
        }
      }
      if (is_frontier && depth > 0) {
        depth--;
      }
      if (next_nyt > 0) {
        push_int_stack(&qs->not_yet_traversed, (int)next_nyt);
      }
    }
  }
  /* end block of split data */
  if (qs->find_split_data) {
    if (!empty_nsi_t_stack(&qs->subtrees_count)) {
      struct node_subtree_info nsi_out_w = pop_nsi_t_stack(&qs->subtrees_count);
      CHECK_ERR(mark_subtree_size(result, nsi_out_w.node_index,
                                  nsi_out_w.node_relative_depth,
                                  nsi_out_w.subtree_size));

      if (!empty_nsi_t_stack(&qs->subtrees_count)) {
        struct node_subtree_info *nsi_rep_within =
            top_ref_nsi_t_stack(&qs->subtrees_count);
        nsi_rep_within->subtree_size += nsi_out_w.subtree_size;
      }
    }
  }
  /* end block of split data */

  result->child_preorder = current_node_index;
  result->node_relative_depth = depth;

  return SUCCESS_ECODE_K2T;
}

int find_point(struct block *input_block, struct queries_state *qs,
               struct point_search_result *psr, TREE_DEPTH_T block_depth) {
  struct child_result current_cr;
  clean_child_result(&current_cr);
  current_cr.resulting_block = input_block;
  current_cr.block_depth = block_depth;

  uint32_t depth = block_depth;
  uint32_t relative_depth = 0;
  for (depth = block_depth; depth < qs->treedepth; depth++) {
    relative_depth = depth - current_cr.block_depth;
    struct child_result prev_cr = current_cr;
    uint32_t current_mcode = get_code_at_morton_code(&qs->mc, depth);

    int child_err_code = child(
        current_cr.resulting_block, current_cr.resulting_node_idx,
        current_mcode, relative_depth, &current_cr, qs, current_cr.block_depth);
    if (child_err_code != 0 && child_err_code != DOES_NOT_EXIST_CHILD_ERR) {
      return child_err_code;
    }

    if (current_cr.check_frontier && !current_cr.exists &&
        current_cr.resulting_block != NULL &&
        current_cr.resulting_block != input_block) {
      prev_cr.previous_block = prev_cr.resulting_block;
      prev_cr.resulting_block = current_cr.resulting_block;
      prev_cr.block_depth = current_cr.block_depth;
      prev_cr.went_frontier = current_cr.went_frontier;

      prev_cr.previous_depth = prev_cr.resulting_relative_depth;
      prev_cr.previous_preorder = prev_cr.resulting_node_idx;
      prev_cr.previous_to_current_index = current_mcode;

      prev_cr.resulting_node_idx = 0;
      prev_cr.resulting_relative_depth = 0;
    }

    psr->treedepth = qs->treedepth;

    if (!current_cr.exists) {
      psr->last_child_result_reached = prev_cr;
      psr->depth_reached = depth;
      psr->point_exists = FALSE;
      return SUCCESS_ECODE_K2T;
    }

    if (current_cr.is_leaf_result) {
      uint32_t leaf_code = leaf_child_morton_code(&qs->mc);

      int does_child_exists;
      _SAFE_OP_K2(child_exists(current_cr.resulting_block,
                               current_cr.resulting_node_idx, leaf_code,
                               &does_child_exists));
      psr->last_child_result_reached = current_cr;
      psr->depth_reached = depth + 1;
      psr->point_exists = does_child_exists;
      return SUCCESS_ECODE_K2T;
    }
  }

  // If the loop ends, then we have found a point
  psr->last_child_result_reached = current_cr;
  psr->depth_reached = depth - 1;
  psr->point_exists = TRUE;

  return SUCCESS_ECODE_K2T;
}

int find_insertion_location(struct block *input_block, struct queries_state *qs,
                            struct insertion_location *result,
                            TREE_DEPTH_T block_depth) {
  struct point_search_result psr;
  CHECK_ERR(find_point(input_block, qs, &psr, block_depth));

  struct block *reached_block = psr.last_child_result_reached.resulting_block;
  TREE_DEPTH_T reached_block_depth = psr.last_child_result_reached.block_depth;

  uint32_t frontier_traversal_idx = 0;
  uint32_t node_index = psr.last_child_result_reached.resulting_node_idx;

  result->parent_node = psr;

  if (psr.point_exists || psr.depth_reached == qs->treedepth - 1 ||
      psr.last_child_result_reached.is_leaf_result) {
    result->insertion_index = node_index;
    result->remaining_depth = 0;
    return SUCCESS_ECODE_K2T;
  }

  int is_frontier;
#ifdef DEBUG_STATS
  struct timeval tval_before, tval_after, tval_result;
  gettimeofday(&tval_before, NULL);
#endif
  is_frontier =
      frontier_check(reached_block, node_index, &frontier_traversal_idx);
#ifdef DEBUG_STATS
  gettimeofday(&tval_after, NULL);
  timersub(&tval_after, &tval_before, &tval_result);
  qs->dstats.time_on_frontier_check +=
      (unsigned long)tval_result.tv_sec * 1000000 + tval_result.tv_usec;
#endif
  if (is_frontier) {
    return FRONTIER_NODE_WITHIN_FIND_INSERTION_LOC;
  }

  uint32_t child_code = get_code_at_morton_code(&qs->mc, psr.depth_reached);

  uint32_t to_be_skipped_subtrees = get_previous_siblings_count(
      reached_block, &psr.last_child_result_reached, child_code);

  if (to_be_skipped_subtrees == 0 && psr.depth_reached == 0) {
    if (get_nodes_count(reached_block) == 0) {
      result->insertion_index = 0;
      result->remaining_depth = qs->treedepth - psr.depth_reached;
    } else {
      result->insertion_index = 1;
      result->remaining_depth = qs->treedepth - psr.depth_reached - 1;
    }
    return SUCCESS_ECODE_K2T;
  }

  qs->find_split_data = FALSE;
#ifdef DEBUG_STATS
  gettimeofday(&tval_before, NULL);
#endif
  CHECK_ERR(sequential_scan_child(
      reached_block, node_index, to_be_skipped_subtrees,
      &frontier_traversal_idx, psr.depth_reached - reached_block_depth, qs,
      reached_block_depth));
#ifdef DEBUG_STATS
  gettimeofday(&tval_after, NULL);
  timersub(&tval_after, &tval_before, &tval_result);
  qs->dstats.time_on_sequential_scan +=
      (unsigned long)tval_result.tv_sec * 1000000 + tval_result.tv_usec;
#endif

  result->insertion_index = qs->sc_result.child_preorder + 1;
  result->remaining_depth = qs->treedepth - 1 - psr.depth_reached;

  return SUCCESS_ECODE_K2T;
}

int get_previous_siblings_count(struct block *input_block,
                                struct child_result *parent_node_result,
                                uint32_t child_code) {
  if (parent_node_result->resulting_node_idx >= get_nodes_count(input_block)) {
    return 0;
  }
  return get_subtree_skipping_qty(
      input_block, parent_node_result->resulting_node_idx, child_code);
}
/**
 * @brief Makes room for a suffix-path to be inserted
 *
 *
 * @param input_block
 * @param il
 * @return int
 */
int make_room(struct block *input_block, struct insertion_location *il) {
  uint32_t next_node_index = il->insertion_index;
  uint32_t nodes_to_insert = il->remaining_depth;
  uint32_t occupied_nodes = get_nodes_count(input_block);

  if (nodes_to_insert == 0) {
    return SUCCESS_ECODE_K2T;
  }

  if (occupied_nodes <= next_node_index) {
    CHECK_ERR(
        enlarge_block_size_to(input_block, occupied_nodes + nodes_to_insert));
    uint32_t nodes_count = get_nodes_count(input_block);
    set_nodes_count(input_block, nodes_count + nodes_to_insert);
  } else {
    CHECK_ERR(shift_right_nodes_after(input_block, next_node_index - 1,
                                      nodes_to_insert));
  }

  return SUCCESS_ECODE_K2T;
}
/**
 * @brief Inserts a point from the given morton code 'mc' and insertion location
 * 'il'
 *
 * Actually, this completes the path for the point encoded in 'mc' to exist,
 * assuming that a prefix-path exists until the location given in 'il'
 *
 * @param input_block The block in which the full suffix-path of nodes will be
 * inserted
 * @param mc Morton code encoding fully the point the be inserted
 * @param il Specifies the exact location where the suffix-path should start
 * @return int Result code
 */
int insert_point_mc(struct block *input_block, struct morton_code *mc,
                    struct insertion_location *il) {
  uint32_t current_index;
  if (il->remaining_depth == mc->treedepth - 1 && il->insertion_index == 0) {
    current_index = 0;
  } else {
    current_index = il->insertion_index;
  }

  for (uint32_t code_idx = mc->treedepth - il->remaining_depth;
       code_idx < mc->treedepth; code_idx++) {
    uint32_t code = get_code_at_morton_code(mc, code_idx);
    CHECK_ERR(insert_node_at(input_block, current_index++, code));
  }

  CHECK_ERR(fix_frontier_indexes(input_block, il->insertion_index,
                                 -((int)il->remaining_depth)));

  return SUCCESS_ECODE_K2T;
}
/**
 * @brief Creates a new block that becomes child of the given one
 *
 * @param input_block Block from where to extract the new sub-block
 * @param from Starting location in input_block of the new block
 * @param to End location in input_block of the new block (inclusive)
 * @param relative_depth Relative depth of the new block w/r to the tree
 * @param new_block New block to be created
 * @return int Result code
 */
int make_new_block(struct block *input_block, uint32_t from, uint32_t to,
                   struct block *new_block) {
  // struct block *created_block = create_block();

  /* initialize block topology */
  CHECK_ERR(init_block_topology(new_block, to - from + 1));
  uint32_t new_bv_start_pos = 4 * from;
  uint32_t new_bv_end_pos = 4 * (to + 1) - 1;
  CHECK_ERR(extract_sub_bitvector(input_block, new_bv_start_pos, new_bv_end_pos,
                                  new_block));
  /* shrink parent bitvector */
  CHECK_ERR(collapse_nodes(input_block, from + 1, to));
  /* initialize block frontier */
  CHECK_ERR(extract_sub_block_frontier(input_block, from, to, new_block));

  //*new_block = created_block;

  return SUCCESS_ECODE_K2T;
}
/**
 * @brief Splits the given block
 *
 * The splitting criteria is hardcoded to select the leftmost node
 * which is not already a frontier node and has a subtree of size at least
 * 1/4 of the size of the block
 *
 * @param input_block Block to split
 * @param qs Used to gather sizes of each subtree in a single scan
 * @return int
 */
int split_block(struct block *input_block, struct queries_state *qs,
                TREE_DEPTH_T block_depth) {
  /* find split location */
  uint32_t new_frontier_node_position;
  uint32_t new_frontier_node_relative_depth;

  uint32_t traversal_frontier_idx = 0;
  uint32_t children_count;

  CHECK_ERR(count_children(input_block, 0, &children_count));
  qs->find_split_data = TRUE;
#ifdef DEBUG_STATS
  struct timeval tval_before, tval_after, tval_result;
  gettimeofday(&tval_before, NULL);
#endif
  CHECK_ERR(sequential_scan_child(input_block, 0, children_count,
                                  &traversal_frontier_idx, 0, qs, block_depth));
#ifdef DEBUG_STATS
  gettimeofday(&tval_after, NULL);
  timersub(&tval_after, &tval_before, &tval_result);
  qs->dstats.time_on_sequential_scan +=
      (unsigned long)tval_result.tv_sec * 1000000 + tval_result.tv_usec;
#endif

  qs->find_split_data = FALSE;

  int leftmost_not_frontier = -1;
  int leftmost_not_frontier_depth = -1;
  int is_frontier = FALSE;
  uint32_t frontier_traversal_index = 0;

  int found_loc = FALSE;
  for (uint32_t node_index = 1; node_index <= qs->sc_result.child_preorder;
       node_index++) {
#ifdef DEBUG_STATS
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
#endif
    is_frontier =
        frontier_check(input_block, node_index, &frontier_traversal_index);
#ifdef DEBUG_STATS
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    qs->dstats.time_on_frontier_check +=
        (unsigned long)tval_result.tv_sec * 1000000 + tval_result.tv_usec;
#endif
    if (is_frontier) {
      continue;
    } else {
      if (leftmost_not_frontier == -1) {
        leftmost_not_frontier = node_index;
        leftmost_not_frontier_depth =
            qs->sc_result.relative_depth_map[node_index];
      }
    }

    uint32_t subtree_size = qs->sc_result.subtrees_count_map[node_index];
    if (subtree_size >= get_nodes_count(input_block) / 4) {
      new_frontier_node_position = node_index;
      new_frontier_node_relative_depth =
          qs->sc_result.relative_depth_map[node_index];
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
  CHECK_ERR(
      count_children(input_block, new_frontier_node_position, &children_count));
  qs->find_split_data = FALSE;
#ifdef DEBUG_STATS
  gettimeofday(&tval_before, NULL);
#endif
  CHECK_ERR(sequential_scan_child(input_block, new_frontier_node_position,
                                  children_count, &traversal_frontier_idx,
                                  new_frontier_node_relative_depth, qs,
                                  block_depth));
#ifdef DEBUG_STATS
  gettimeofday(&tval_after, NULL);
  timersub(&tval_after, &tval_before, &tval_result);
  qs->dstats.time_on_sequential_scan +=
      (unsigned long)tval_result.tv_sec * 1000000 + tval_result.tv_usec;
#endif

  uint32_t right_index = qs->sc_result.child_preorder;

  struct block new_block;
  new_block.children = 0;
  new_block.children_blocks = NULL;
  new_block.nodes_count = 0;
  new_block.container_size = 0;
  new_block.container = NULL;
  new_block.preorders = NULL;
  CHECK_ERR(make_new_block(input_block, new_frontier_node_position, right_index,
                           &new_block));

  CHECK_ERR(
      fix_frontier_indexes(&new_block, 0, new_frontier_node_position + 1));

  int delta_indexes_parent = right_index - new_frontier_node_position;

  CHECK_ERR(fix_frontier_indexes(input_block, new_frontier_node_position + 1,
                                 delta_indexes_parent));

  CHECK_ERR(
      add_frontier_node(input_block, new_frontier_node_position, &new_block));

  return SUCCESS_ECODE_K2T;
}

/**
 * @brief Inserts a point encoded in qs->mc to the tree
 *
 * It's intended used is private and should be used through insert_point
 * function
 *
 * First checks if the block has enough space, if not, first check if the block
 * can be expanded to a size lesser than the maximum permitted, if that is the
 * case expand and then retry inserting (recursively)
 *
 * If the next possible size is greater than the maximum permitted, splits the
 * block and retry (recursively)
 *
 * @param insertion_block  The block in which the point would be inserted
 * @param il Describes the location at where the point would be inserted
 * @param qs Describes the point to be inserted and auxilliary data
 * @return int error code
 */
int insert_point_at(struct block *insertion_block,
                    struct insertion_location *il, struct queries_state *qs,
                    TREE_DEPTH_T block_depth, int *already_existed) {
  int aux_was_set;
  if (block_has_enough_space(insertion_block, il)) {
    CHECK_ERR(make_room(insertion_block, il));
    struct child_result *lcresult =
        &(il->parent_node).last_child_result_reached;
    int child_node_is_parent =
        il->insertion_index == lcresult->resulting_node_idx;
    if (!child_node_is_parent) {
      uint32_t node_code =
          get_code_at_morton_code(&qs->mc, il->parent_node.depth_reached);
      CHECK_ERR(mark_child_in_node(
          insertion_block,
          il->parent_node.last_child_result_reached.resulting_node_idx,
          node_code, &aux_was_set));

      struct block *previous_block = lcresult->previous_block;
      if (lcresult->went_frontier) {
        CHECK_ERR(mark_child_in_node(
            previous_block, lcresult->previous_preorder,
            lcresult->previous_to_current_index, &aux_was_set));
      }
    }

    if (il->remaining_depth == 0) {
      uint32_t leaf_child = leaf_child_morton_code(&qs->mc);
      CHECK_ERR(mark_child_in_node(insertion_block, il->insertion_index,
                                   leaf_child, already_existed));
    } else {
      CHECK_ERR(insert_point_mc(insertion_block, &qs->mc, il));
      *already_existed = FALSE;
    }

    return SUCCESS_ECODE_K2T;
  }

  // Check if can enlarge block to fit
  uint32_t next_amount_of_nodes =
      get_nodes_count(insertion_block) + il->remaining_depth;
  if (next_amount_of_nodes <= qs->max_nodes_count) {
    uint32_t next_block_sz = 1 << (uint32_t)ceil(log2(next_amount_of_nodes));
    CHECK_ERR(enlarge_block_size_to(insertion_block, next_block_sz));
    return insert_point_at(insertion_block, il, qs, block_depth,
                           already_existed);
  }

  CHECK_ERR(split_block(insertion_block, qs, block_depth));

  // printf("after split...\n");
  // debug_print_block_rec(qs->root);
#ifdef DEBUG_STATS
  qs->dstats.split_count++;
#endif

  struct insertion_location il_split;
  CHECK_ERR(find_insertion_location(qs->root, qs, &il_split, 0));

  return insert_point_at(
      il_split.parent_node.last_child_result_reached.resulting_block, &il_split,
      qs, il_split.parent_node.last_child_result_reached.block_depth,
      already_existed);
}

/**
 * @brief Recursive function to scan for all the points in the tree
 *
 * The scan is performed in a preorder dfs fashion
 *
 * @param input_block Current block to scan recursively
 * @param qs queries state struct
 * @param result Vector storing the points in struct pair2dl each
 * @param cresult Stores info about the current node
 * @return int Result code
 */
int naive_scan_points_rec(struct block *input_block, struct queries_state *qs,
                          struct vector_pair2dl_t *result,
                          struct child_result *cresult,
                          TREE_DEPTH_T block_depth) {
  TREE_DEPTH_T real_depth = cresult->resulting_relative_depth + block_depth;

  for (uint32_t child_pos = 0; child_pos < 4; child_pos++) {
    if (real_depth == qs->treedepth - 1) {
      int does_child_exist;
      child_exists(input_block, cresult->resulting_node_idx, child_pos,
                   &does_child_exist);
      if (does_child_exist) {
        struct pair2dl pair;
        add_element_morton_code(&qs->mc, real_depth, child_pos);
        convert_morton_code_to_coordinates(&qs->mc, &pair);
        vector_pair2dl_t__insert_element(result, pair);
      }
      continue;
    }

    struct child_result cr;
    int err = child(input_block, cresult->resulting_node_idx, child_pos,
                    cresult->resulting_relative_depth, &cr, qs, block_depth);

    if (err == DOES_NOT_EXIST_CHILD_ERR) {
      continue;
    } else if (err != 0) {
      return err;
    }
    add_element_morton_code(&qs->mc, real_depth, child_pos);
    naive_scan_points_rec(cr.resulting_block, qs, result, &cr, cr.block_depth);
  }

  return SUCCESS_ECODE_K2T;
}

/**
 * @brief Recursive function to scan for all the points in the tree
 * interactively
 *
 * The scan is perfommed in a preorder dfs fashion
 *
 * @param input_block Current block to scan recursively
 * @param qs queries state struct
 * @param result Vector storing the points in struct pair2dl each
 * @param cresult Stores info about the current node
 * @return int Result code
 */
int naive_scan_points_rec_interactively(struct block *input_block,
                                        struct queries_state *qs,
                                        point_reporter_fun_t point_reporter,
                                        void *report_state,
                                        struct child_result *cresult,
                                        TREE_DEPTH_T block_depth) {
  TREE_DEPTH_T real_depth = cresult->resulting_relative_depth + block_depth;

  for (uint32_t child_pos = 0; child_pos < 4; child_pos++) {
    if (real_depth == qs->treedepth - 1) {
      int does_child_exist;
      child_exists(input_block, cresult->resulting_node_idx, child_pos,
                   &does_child_exist);
      if (does_child_exist) {
        struct pair2dl pair;
        add_element_morton_code(&qs->mc, real_depth, child_pos);
        convert_morton_code_to_coordinates(&qs->mc, &pair);
        point_reporter(pair.col, pair.row, report_state);
      }
      continue;
    }

    struct child_result cr;
    int err = child(input_block, cresult->resulting_node_idx, child_pos,
                    cresult->resulting_relative_depth, &cr, qs, block_depth);

    if (err == DOES_NOT_EXIST_CHILD_ERR) {
      continue;
    } else if (err != 0) {
      return err;
    }
    add_element_morton_code(&qs->mc, real_depth, child_pos);
    naive_scan_points_rec_interactively(cr.resulting_block, qs, point_reporter,
                                        report_state, &cr, cr.block_depth);
  }

  return SUCCESS_ECODE_K2T;
}

/**
 * @brief Recursive function to scan for points in a row or column
 *
 * DEV NOTE: Might want to implement this using report_rec_interactively
 * instead of duplicating code
 *
 * @param current_col Column in the current search space
 * @param qs Used to store partial morton code and create coordinate from it
 * when reaching a leaf
 * @param result Used to store the results in struct pair2dl
 * @param current_cr Used to perform child() operation and store info about the
 * current node
 * @param which_report Specifies if this is a column or row search (Possible
 * values: REPORT_COLUMN, REPORT_ROW)
 * @return int Result code
 */
int report_rec(unsigned long current_col, struct queries_state *qs,
               struct vector_pair2dl_t *result, struct child_result *current_cr,
               int which_report) {
  struct block *current_block = current_cr->resulting_block;
  TREE_DEPTH_T current_block_depth = current_cr->block_depth;
  TREE_DEPTH_T tree_depth = qs->treedepth;
  TREE_DEPTH_T relative_depth = current_cr->resulting_relative_depth;
  TREE_DEPTH_T real_depth = relative_depth + current_block_depth;
  if (real_depth + 1 == tree_depth) {
    // report the coordinate, because we have reached a leaf

    unsigned long half_length =
        1UL << ((unsigned long)tree_depth - (unsigned long)real_depth - 1);

    for (uint32_t child_pos = 0; child_pos < 4; child_pos++) {
      if (!REPORT_CONTINUE_CONDITION(current_col, half_length, which_report,
                                     child_pos)) {
        continue;
      }
      int does_child_exist;
      child_exists(current_block, current_cr->resulting_node_idx, child_pos,
                   &does_child_exist);
      if (does_child_exist) {
        struct pair2dl pair;
        add_element_morton_code(&qs->mc, real_depth, child_pos);
        convert_morton_code_to_coordinates(&qs->mc, &pair);
        vector_pair2dl_t__insert_element(result, pair);
      }
    }
    return SUCCESS_ECODE_K2T;
  }

  unsigned long half_length =
      1UL << ((unsigned long)tree_depth - (unsigned long)real_depth - 1UL);

  uint32_t current_node_index = current_cr->resulting_node_idx;

  // This for-loop is a trick to avoid a lot of code duplication.
  // Essentially, when which_report = REPORT_COLUMN, look for the 0th and 1st
  // child if current_col < half_length or for the 2nd and 3rd child if
  // current_col >= half_length. For REPORT_ROW it's but instead of the left and
  // right side of the k2tree matrix, use the top and bottom sides.
  struct child_result next_cr;
  for (uint32_t child_pos = 0; child_pos < 4; child_pos++) {
    if (!REPORT_CONTINUE_CONDITION(current_col, half_length, which_report,
                                   child_pos)) {
      continue;
    }

    // We need to restore next_cr because each time it will be polluted by child
    next_cr = *current_cr;
    CHECK_CHILD_ERR(child(current_block, current_node_index, child_pos,
                          relative_depth, &next_cr, qs, current_block_depth));
    if (next_cr.exists) {
      // We don't need to clean up the morton code because we are traversing in
      // preorder-dfs and always writing in a random access fashion
      add_element_morton_code(&qs->mc, real_depth, child_pos);
      // We take modulo here because the search space is reduced by half
      CHECK_ERR(report_rec(current_col % half_length, qs, result, &next_cr,
                           which_report));
    }
  }

  return SUCCESS_ECODE_K2T;
}

/**
 * @brief Recursive function to scan for points in a row or column interactively
 *
 *
 *
 * @param current_col Column in the current search space
 * @param qs Used to store partial morton code and create coordinate from it
 * when reaching a leaf
 * @param point_reporter Function that receives a pair row, column reporting a
 * point
 * @param current_cr Used to perform child() operation and store info about the
 * current node
 * @param which_report Specifies if this is a column or row search (Possible
 * values: REPORT_COLUMN, REPORT_ROW)
 * @return int Result code
 */
int report_rec_interactively(unsigned long current_col,
                             struct queries_state *qs,
                             point_reporter_fun_t point_reporter,
                             struct child_result *current_cr, int which_report,
                             void *report_state) {
  struct block *current_block = current_cr->resulting_block;
  TREE_DEPTH_T current_block_depth = current_cr->block_depth;
  TREE_DEPTH_T tree_depth = qs->treedepth;
  TREE_DEPTH_T relative_depth = current_cr->resulting_relative_depth;
  TREE_DEPTH_T real_depth = relative_depth + current_block_depth;
  if (real_depth + 1 == tree_depth) {
    // report the coordinate, because we have reached a leaf
    unsigned long half_length =
        1UL << ((unsigned long)tree_depth - (unsigned long)real_depth - 1UL);
    for (uint32_t child_pos = 0; child_pos < 4; child_pos++) {
      if (!REPORT_CONTINUE_CONDITION(current_col, half_length, which_report,
                                     child_pos)) {
        continue;
      }
      int does_child_exist;
      child_exists(current_block, current_cr->resulting_node_idx, child_pos,
                   &does_child_exist);
      if (does_child_exist) {
        struct pair2dl pair;
        add_element_morton_code(&qs->mc, real_depth, child_pos);
        convert_morton_code_to_coordinates(&qs->mc, &pair);
        point_reporter(pair.col, pair.row, report_state);
      }
    }
    return SUCCESS_ECODE_K2T;
  }

  unsigned long half_length =
      1UL << ((unsigned long)tree_depth - (unsigned long)real_depth - 1UL);

  uint32_t current_node_index = current_cr->resulting_node_idx;

  // This for-loop is a trick to avoid a lot of code duplication.
  // Essentially, when which_report = REPORT_COLUMN, look for the 0th and 1st
  // child if current_col < half_length or for the 2nd and 3rd child if
  // current_col >= half_length. For REPORT_ROW it's but instead of the left and
  // right side of the k2tree matrix, use the top and bottom sides.
  struct child_result next_cr;
  for (uint32_t child_pos = 0; child_pos < 4; child_pos++) {
    if (!REPORT_CONTINUE_CONDITION(current_col, half_length, which_report,
                                   child_pos)) {
      continue;
    }

    // We need to restore next_cr because each time it will be polluted by child
    next_cr = *current_cr;
    CHECK_CHILD_ERR(child(current_block, current_node_index, child_pos,
                          relative_depth, &next_cr, qs, current_block_depth));
    if (next_cr.exists) {
      // We don't need to clean up the morton code because we are traversing in
      // preorder-dfs and always writing in a random access fashion
      add_element_morton_code(&qs->mc, real_depth, child_pos);
      // We take modulo here because the search space is reduced by half
      CHECK_ERR(report_rec_interactively(current_col % half_length, qs,
                                         point_reporter, &next_cr, which_report,
                                         report_state));
    }
  }

  return SUCCESS_ECODE_K2T;
}

/* END PRIVATE FUNCTIONS IMPLEMENTATIONS */

/* PUBLIC FUNCTIONS */

int has_point(struct block *input_block, unsigned long col, unsigned long row,
              struct queries_state *qs, int *result) {

  convert_coordinates_to_morton_code(col, row, qs->treedepth, &qs->mc);

  struct point_search_result psr;
  CHECK_ERR(find_point(input_block, qs, &psr, 0));

  *result = psr.point_exists;

  return SUCCESS_ECODE_K2T;
}

int insert_point(struct block *input_block, unsigned long col,
                 unsigned long row, struct queries_state *qs,
                 int *already_exists) {
  convert_coordinates_to_morton_code(col, row, qs->treedepth, &qs->mc);
  struct insertion_location il;
  CHECK_ERR(find_insertion_location(input_block, qs, &il, 0));
  struct block *insertion_block =
      il.parent_node.last_child_result_reached.resulting_block;
  TREE_DEPTH_T insertion_block_depth =
      il.parent_node.last_child_result_reached.block_depth;
  return insert_point_at(insertion_block, &il, qs, insertion_block_depth,
                         already_exists);
}

int naive_scan_points(struct block *input_block, struct queries_state *qs,
                      struct vector_pair2dl_t *result) {
  struct child_result cresult;
  clean_child_result(&cresult);
  return naive_scan_points_rec(input_block, qs, result, &cresult, 0);
}

int scan_points_interactively(struct block *input_block,
                              struct queries_state *qs,
                              point_reporter_fun_t point_reporter,
                              void *report_state) {
  struct child_result cresult;
  clean_child_result(&cresult);
  return naive_scan_points_rec_interactively(input_block, qs, point_reporter,
                                             report_state, &cresult, 0);
}

int report_column(struct block *input_block, unsigned long col,
                  struct queries_state *qs, struct vector_pair2dl_t *result) {
  struct child_result current_cr;
  clean_child_result(&current_cr);
  current_cr.resulting_block = input_block;
  current_cr.block_depth = 0;
  return report_rec(col, qs, result, &current_cr, REPORT_COLUMN);
}

int report_row(struct block *input_block, unsigned long row,
               struct queries_state *qs, struct vector_pair2dl_t *result) {
  struct child_result current_cr;
  clean_child_result(&current_cr);
  current_cr.resulting_block = input_block;
  current_cr.block_depth = 0;
  return report_rec(row, qs, result, &current_cr, REPORT_ROW);
}

int report_column_interactively(struct block *input_block, unsigned long col,
                                struct queries_state *qs,
                                point_reporter_fun_t point_reporter,
                                void *report_state) {
  struct child_result current_cr;
  clean_child_result(&current_cr);
  current_cr.resulting_block = input_block;
  current_cr.block_depth = 0;
  return report_rec_interactively(col, qs, point_reporter, &current_cr,
                                  REPORT_COLUMN, report_state);
}

int report_row_interactively(struct block *input_block, unsigned long row,
                             struct queries_state *qs,
                             point_reporter_fun_t point_reporter,
                             void *report_state) {
  struct child_result current_cr;
  clean_child_result(&current_cr);
  current_cr.resulting_block = input_block;
  current_cr.block_depth = 0;
  return report_rec_interactively(row, qs, point_reporter, &current_cr,
                                  REPORT_ROW, report_state);
}

struct block *create_block(void) {
  struct block *new_block = k2tree_alloc_block();
  new_block->container = NULL;
  new_block->container_size = 0;
  new_block->children_blocks = NULL;
  new_block->preorders = NULL;
  new_block->nodes_count = 0;
  new_block->children = 0;
  create_block_topology(new_block);
  init_block_frontier(new_block);

  return new_block;
}

int free_rec_block(struct block *input_block) {
  CHECK_ERR(free_rec_block_internal(input_block));
  k2tree_free_block(input_block);
  return SUCCESS_ECODE_K2T;
}

int free_rec_block_internal(struct block *input_block) {
  for (int i = 0; i < (int)input_block->children; i++) {
    struct block *current_block = &input_block->children_blocks[i];
    CHECK_ERR(free_rec_block_internal(current_block));
  }

  CHECK_ERR(free_block(input_block));
  return SUCCESS_ECODE_K2T;
}

int free_block(struct block *input_block) {
  CHECK_ERR(free_block_topology(input_block));
  free_block_frontier(input_block);
  // k2tree_free_blocks_array(input_block);
  // k2tree_free_block(input_block);
  return SUCCESS_ECODE_K2T;
}

struct k2tree_measurement measure_tree_size(struct block *input_block) {
  unsigned long children_total_bytes = 0;
  unsigned long children_total_blocks = 0;
  unsigned long children_bytes_topology = 0;

  for (int child_block_index = 0;
       child_block_index < (int)input_block->children; child_block_index++) {
    struct k2tree_measurement children_measurement =
        measure_tree_size(&input_block->children_blocks[child_block_index]);
    children_total_bytes += children_measurement.total_bytes;
    children_total_blocks += children_measurement.total_blocks;
    children_bytes_topology += children_measurement.bytes_topology;
  }

  unsigned long bytes_topology = input_block->container_size * sizeof(BVCTYPE);

  unsigned long block_total_bytes = sizeof(struct block) + bytes_topology +
                                    input_block->children * (sizeof(uint32_t));

  struct k2tree_measurement result;
  result.total_bytes = block_total_bytes + children_total_bytes;
  result.bytes_topology = bytes_topology + children_bytes_topology;
  result.total_blocks = 1 + children_total_blocks;
  return result;
}

static inline void sip_select_child(unsigned long coord, coord_t coord_type,
                                    int *selected_child_1,
                                    int *selected_child_2,
                                    unsigned long half_length) {
  switch (coord_type) {
  case COLUMN_COORD:
    if (coord < half_length) {
      *selected_child_1 = 0;
      *selected_child_2 = 1;
    } else {
      *selected_child_1 = 2;
      *selected_child_2 = 3;
    }
    break;
  case ROW_COORD:
  default:
    if (coord < half_length) {
      *selected_child_1 = 0;
      *selected_child_2 = 2;
    } else {
      *selected_child_1 = 1;
      *selected_child_2 = 3;
    }
    break;
  }
}

static inline int sip_assign_valid_part(int *valid_part_1, int *valid_part_2,
                                        int selected_child_1,
                                        int selected_child_2, int index,
                                        struct child_result *crs) {
  if (*valid_part_1)
    CHECK_ERR(child_exists(crs[index].resulting_block,
                           crs[index].resulting_node_idx, selected_child_1,
                           valid_part_1));
  if (*valid_part_2)
    CHECK_ERR(child_exists(crs[index].resulting_block,
                           crs[index].resulting_node_idx, selected_child_2,
                           valid_part_2));
  return SUCCESS_ECODE_K2T;
}

static int sip_join_rec(struct sip_join_input input,
                        coord_reporter_fun_t coord_reporter, void *report_state,
                        struct child_result *crs, struct morton_code *mc,
                        TREE_DEPTH_T current_depth) {
  TREE_DEPTH_T current_block_depth =
      crs[current_depth * input.join_size].block_depth;
  TREE_DEPTH_T tree_depth = input.qss[0]->treedepth;
  TREE_DEPTH_T relative_depth =
      crs[current_depth * input.join_size].resulting_relative_depth;
  TREE_DEPTH_T real_depth = relative_depth + current_block_depth;
  unsigned long half_length =
      1UL << ((unsigned long)tree_depth - (unsigned long)real_depth - 1UL);

  int valid_parts[2] = {TRUE, TRUE};
  int selected_children[2];
  for (int i = 0; i < input.join_size; i++) {
    int join_index = current_depth * input.join_size + i;
    sip_select_child(input._join_coords[join_index].coord,
                     input._join_coords[join_index].coord_type,
                     &selected_children[0], &selected_children[1], half_length);
    CHECK_ERR(sip_assign_valid_part(&valid_parts[0], &valid_parts[1],
                                    selected_children[0], selected_children[1],
                                    current_depth * input.join_size + i, crs));
  }

  for (int i = 0; i < 2; i++) {
    if (valid_parts[i]) {
      // Will always be the last given band
      add_element_morton_code(mc, real_depth, selected_children[i]);
      if (real_depth + 1 == tree_depth) {
        struct pair2dl pair;
        convert_morton_code_to_coordinates(mc, &pair);
        switch (input.join_coords[input.join_size - 1].coord_type) {
        case COLUMN_COORD:
          coord_reporter(pair.row, report_state);
          break;
        case ROW_COORD:
        default:
          coord_reporter(pair.col, report_state);
          break;
        }
      } else {
        for (int j = 0; j < input.join_size; j++) {
          int join_index = current_depth * input.join_size + j;
          int next_depth_join_index = (current_depth + 1) * input.join_size + j;
          sip_select_child(input._join_coords[join_index].coord,
                           input._join_coords[join_index].coord_type,
                           &selected_children[0], &selected_children[1],
                           half_length);

          struct child_result next_cr =
              crs[current_depth * input.join_size + j];
          CHECK_ERR(child(next_cr.resulting_block, next_cr.resulting_node_idx,
                          selected_children[i],
                          next_cr.resulting_relative_depth, &next_cr,
                          input.qss[j], next_cr.block_depth));
          crs[next_depth_join_index] = next_cr;
          input._join_coords[next_depth_join_index].coord_type =
              input._join_coords[join_index].coord_type;
          input._join_coords[next_depth_join_index].coord =
              input._join_coords[join_index].coord % half_length;

          if (!next_cr.exists) {
            // debug
            fprintf(stderr, "!next_cr.exists");
            exit(1);
          }
        }

        CHECK_ERR(sip_join_rec(input, coord_reporter, report_state, crs, mc,
                               current_depth + 1));
      }
    }
  }

  return SUCCESS_ECODE_K2T;
}

int sip_join(struct sip_join_input input, coord_reporter_fun_t coord_reporter,
             void *report_state) {
  TREE_DEPTH_T treedepth = input.qss[0]->treedepth;
  struct child_result *crs =
      malloc(input.join_size * treedepth * sizeof(struct child_result));
  input._join_coords =
      malloc(input.join_size * treedepth * sizeof(struct sip_ipoint));
  for (int j = 0; j < treedepth; j++) {
    for (int i = 0; i < input.join_size; i++) {
      int index = j * input.join_size + i;
      clean_child_result(crs + index);
      crs[index].resulting_block = input.blocks[i];
    }
  }

  for (int i = 0; i < input.join_size; i++) {
    input.qss[i]->root = input.blocks[i];
    input._join_coords[i] = input.join_coords[i];
  }

  struct morton_code mc;
  init_morton_code(&mc, treedepth);

  CHECK_ERR(sip_join_rec(input, coord_reporter, report_state, crs, &mc, 0));

  free(crs);
  free(input._join_coords);
  clean_morton_code(&mc);

  return SUCCESS_ECODE_K2T;
}

// Returns 0 when the block is valid
int debug_validate_block(struct block *input_block) {
  for (int node_i = 0; node_i < (int)input_block->nodes_count; node_i++) {
    int container_i = (node_i * 4) / 32;
    uint32_t part = input_block->container[container_i];
    int node_in_part = (node_i * 4) % 32;
    int empty_node = TRUE;
    for (int j = 0; j < 4; j++) {
      if (part & (1 << (31 - (node_in_part + j)))) {
        empty_node = FALSE;
        break;
      }
    }
    if (empty_node)
      return 1;
  }
  return 0;
}

int debug_validate_block_rec(struct block *input_block) {
  if (debug_validate_block(input_block))
    return 1;

  for (int i = 0; i < (int)input_block->children; i++) {
    int res = debug_validate_block_rec(&input_block->children_blocks[i]);
    if (res)
      return res + 1;
  }

  return 0;
}

void debug_print_block(struct block *b) {
  printf("nodes count: %d, container size: %d, block ptr: %p\n", b->nodes_count,
         b->container_size, (void *)b);
  for (unsigned int i = 0; i < b->container_size; i++) {
    if ((i * 32) >= 4 * b->nodes_count)
      break;
    for (unsigned int j = 0; j < 32; j++) {
      if ((i * 32 + j) >= 4 * b->nodes_count)
        break;
      int bit_on = !!(b->container[i] & (1U << (31 - j)));
      if (j % 8 == 0 && j != 0)
        printf(" ");
      printf("%d", bit_on);
    }
    printf("  ");
  }
  printf("\n");

  for (unsigned int i = 0; i < b->children; i++) {
    printf("%d,", b->preorders[i]);
  }
  printf("\n\n");
}

void debug_print_block_rec(struct block *b) {
  debug_print_block(b);
  for (int i = 0; i < (int)b->children; i++) {
    debug_print_block_rec(&b->children_blocks[i]);
  }
}
int naive_scan_points_lazy_init(
    struct block *input_block, struct queries_state *qs,
    struct lazy_handler_naive_scan_t *lazy_handler) {

  init_lazy_naive_state_stack(&lazy_handler->states_stack, qs->treedepth * 4);
  lazy_handler->qs = qs;
  lazy_handler->has_next = FALSE;
  lazy_handler->tree_root = input_block;

  lazy_naive_state first_state;
  first_state.block_depth = 0;
  first_state.input_block = input_block;
  first_state.last_iteration = 0;
  clean_child_result(&first_state.cr);

  push_lazy_naive_state_stack(&lazy_handler->states_stack, first_state);
  naive_scan_points_lazy_next(lazy_handler, &lazy_handler->next_result);

  return SUCCESS_ECODE_K2T;
}

int naive_scan_points_lazy_clean(
    struct lazy_handler_naive_scan_t *lazy_handler) {
  free_lazy_naive_state_stack(&lazy_handler->states_stack);
  return SUCCESS_ECODE_K2T;
}

int naive_scan_points_lazy_next(struct lazy_handler_naive_scan_t *lazy_handler,
                                pair2dl_t *result) {
  *result = lazy_handler->next_result;
  while (!empty_lazy_naive_state_stack(&lazy_handler->states_stack)) {
    lazy_naive_state current =
        pop_lazy_naive_state_stack(&lazy_handler->states_stack);
    struct queries_state *qs = lazy_handler->qs;
    struct block *input_block = current.input_block;
    struct child_result *cresult = &current.cr;

    TREE_DEPTH_T real_depth =
        cresult->resulting_relative_depth + current.block_depth;
    for (uint32_t child_pos = current.last_iteration; child_pos < 4;
         child_pos++) {
      if (real_depth == qs->treedepth - 1) {
        int does_child_exist;
        child_exists(input_block, cresult->resulting_node_idx, child_pos,
                     &does_child_exist);
        if (does_child_exist) {
          add_element_morton_code(&qs->mc, real_depth, child_pos);
          convert_morton_code_to_coordinates(&qs->mc,
                                             &lazy_handler->next_result);
          lazy_naive_state next_state;
          next_state.block_depth = current.block_depth;
          next_state.input_block = current.input_block;
          next_state.cr = current.cr;
          next_state.last_iteration = child_pos + 1;
          lazy_handler->has_next = TRUE;
          push_lazy_naive_state_stack(&lazy_handler->states_stack, next_state);

          return SUCCESS_ECODE_K2T;
        }
        continue;
      }

      struct child_result cr;
      clean_child_result(&cr);
      int err = child(input_block, cresult->resulting_node_idx, child_pos,
                      cresult->resulting_relative_depth, &cr, qs,
                      current.block_depth);

      if (err == DOES_NOT_EXIST_CHILD_ERR) {
        continue;
      } else if (err != 0) {
        return err;
      }
      add_element_morton_code(&qs->mc, real_depth, child_pos);

      if (child_pos < 3) {
        lazy_naive_state sibling_state;
        sibling_state.block_depth = current.block_depth;
        sibling_state.input_block = current.input_block;
        sibling_state.cr = current.cr;
        sibling_state.last_iteration = child_pos + 1;
        push_lazy_naive_state_stack(&lazy_handler->states_stack, sibling_state);
      }

      lazy_naive_state next_state;
      next_state.block_depth = cr.block_depth;
      next_state.input_block = cr.resulting_block;
      next_state.cr = cr;
      next_state.last_iteration = 0;
      push_lazy_naive_state_stack(&lazy_handler->states_stack, next_state);
      break;
    }
  }
  lazy_handler->has_next = FALSE;
  return SUCCESS_ECODE_K2T;
}

int naive_scan_points_lazy_has_next(
    struct lazy_handler_naive_scan_t *lazy_handler, int *result) {
  *result = lazy_handler->has_next;
  return SUCCESS_ECODE_K2T;
}

int naive_scan_points_lazy_reset(
    struct lazy_handler_naive_scan_t *lazy_handler) {
  reset_lazy_naive_state_stack(&lazy_handler->states_stack);

  lazy_handler->has_next = FALSE;

  lazy_naive_state first_state;
  first_state.block_depth = 0;
  first_state.input_block = lazy_handler->tree_root;
  first_state.last_iteration = 0;
  clean_child_result(&first_state.cr);

  push_lazy_naive_state_stack(&lazy_handler->states_stack, first_state);
  naive_scan_points_lazy_next(lazy_handler, &lazy_handler->next_result);

  return SUCCESS_ECODE_K2T;
}

int report_band_next(struct lazy_handler_report_band_t *lazy_handler,
                     uint64_t *result) {
  *result = lazy_handler->next_result;
  struct queries_state *qs = lazy_handler->qs;
  while (!empty_lazy_report_band_state_t_stack(&lazy_handler->stack)) {
    lazy_report_band_state_t current_state =
        pop_lazy_report_band_state_t_stack(&lazy_handler->stack);
    struct child_result *current_cr = &current_state.current_cr;
    struct block *current_block = current_cr->resulting_block;
    TREE_DEPTH_T current_block_depth = current_cr->block_depth;
    TREE_DEPTH_T tree_depth = qs->treedepth;
    TREE_DEPTH_T relative_depth = current_cr->resulting_relative_depth;
    TREE_DEPTH_T real_depth = relative_depth + current_block_depth;

    unsigned long half_length =
        1UL << ((unsigned long)tree_depth - (unsigned long)real_depth - 1);

    for (uint32_t child_pos = current_state.last_iteration; child_pos < 4;
         child_pos++) {
      if (!REPORT_CONTINUE_CONDITION(current_state.current_coord, half_length,
                                     lazy_handler->which_report, child_pos)) {
        continue;
      }

      if (real_depth + 1 == tree_depth) {
        int does_child_exist;
        child_exists(current_block, current_cr->resulting_node_idx, child_pos,
                     &does_child_exist);
        if (does_child_exist) {
          struct pair2dl pair;
          add_element_morton_code(&qs->mc, real_depth, child_pos);
          convert_morton_code_to_coordinates(&qs->mc, &pair);
          if (lazy_handler->which_report == REPORT_COLUMN)
            lazy_handler->next_result = pair.row;
          else
            lazy_handler->next_result = pair.col;

          lazy_report_band_state_t next_state;
          next_state.current_coord = current_state.current_coord;
          next_state.current_cr = current_state.current_cr;
          next_state.last_iteration = child_pos + 1;
          lazy_handler->has_next = TRUE;
          push_lazy_report_band_state_t_stack(&lazy_handler->stack, next_state);
          return SUCCESS_ECODE_K2T;
        }
        continue;
      }
      uint32_t current_node_index = current_cr->resulting_node_idx;
      struct child_result next_cr = *current_cr;
      CHECK_CHILD_ERR(child(current_block, current_node_index, child_pos,
                            relative_depth, &next_cr, qs, current_block_depth));
      if (next_cr.exists) {

        add_element_morton_code(&qs->mc, real_depth, child_pos);
        if (child_pos < 3) {
          lazy_report_band_state_t sibling_state;
          sibling_state.current_coord = current_state.current_coord;
          sibling_state.current_cr = current_state.current_cr;
          sibling_state.last_iteration = child_pos + 1;
          push_lazy_report_band_state_t_stack(&lazy_handler->stack,
                                              sibling_state);
        }
        lazy_report_band_state_t next_state;
        next_state.current_coord = current_state.current_coord % half_length;
        next_state.current_cr = next_cr;
        next_state.last_iteration = 0;
        push_lazy_report_band_state_t_stack(&lazy_handler->stack, next_state);
        break;
      }
    }
  }

  lazy_handler->has_next = FALSE;
  return SUCCESS_ECODE_K2T;
}

int report_band_lazy_init(struct lazy_handler_report_band_t *lazy_handler,
                          struct block *input_block, struct queries_state *qs,
                          int which_report, uint64_t coord);

int report_band_lazy_init(struct lazy_handler_report_band_t *lazy_handler,
                          struct block *input_block, struct queries_state *qs,
                          int which_report, uint64_t coord) {

  lazy_handler->which_report = which_report;
  lazy_handler->has_next = FALSE;
  lazy_handler->qs = qs;
  lazy_handler->coord_to_report = coord;
  lazy_handler->tree_root = input_block;

  init_lazy_report_band_state_t_stack(&lazy_handler->stack,
                                      lazy_handler->qs->treedepth * 4);

  lazy_report_band_state_t first_state;
  first_state.current_coord = coord;
  clean_child_result(&first_state.current_cr);
  first_state.current_cr.resulting_block = input_block;
  first_state.last_iteration = 0;

  push_lazy_report_band_state_t_stack(&lazy_handler->stack, first_state);
  report_band_next(lazy_handler, &lazy_handler->next_result);

  return SUCCESS_ECODE_K2T;
}

int report_column_lazy_init(struct lazy_handler_report_band_t *lazy_handler,
                            struct block *input_block, struct queries_state *qs,
                            uint64_t coord) {
  report_band_lazy_init(lazy_handler, input_block, qs, REPORT_COLUMN, coord);
  return SUCCESS_ECODE_K2T;
}
int report_row_lazy_init(struct lazy_handler_report_band_t *lazy_handler,
                         struct block *input_block, struct queries_state *qs,
                         uint64_t coord) {
  report_band_lazy_init(lazy_handler, input_block, qs, REPORT_ROW, coord);
  return SUCCESS_ECODE_K2T;
}

int report_band_lazy_clean(struct lazy_handler_report_band_t *lazy_handler) {
  free_lazy_report_band_state_t_stack(&lazy_handler->stack);
  return SUCCESS_ECODE_K2T;
}

int report_band_has_next(struct lazy_handler_report_band_t *lazy_handler,
                         int *result) {
  *result = lazy_handler->has_next;
  return SUCCESS_ECODE_K2T;
}

int report_band_reset(struct lazy_handler_report_band_t *lazy_handler) {
  reset_lazy_report_band_state_t_stack(&lazy_handler->stack);
  lazy_handler->has_next = FALSE;

  lazy_report_band_state_t first_state;
  first_state.current_coord = lazy_handler->coord_to_report;
  clean_child_result(&first_state.current_cr);
  first_state.current_cr.resulting_block = lazy_handler->tree_root;
  first_state.last_iteration = 0;

  push_lazy_report_band_state_t_stack(&lazy_handler->stack, first_state);
  report_band_next(lazy_handler, &lazy_handler->next_result);

  return SUCCESS_ECODE_K2T;
}

int delete_nodes_in_block(struct block *input_block, struct deletion_state *ds,
                          int *total_deleted) {

  int amount_to_delete = size_int_stack(&ds->nodes_to_delete);
  if (amount_to_delete >= input_block->nodes_count - 1 &&
      top_int_stack(&ds->nodes_to_delete) != 0) {
    push_int_stack(&ds->nodes_to_delete, 0);
    amount_to_delete++;
  }

  if (empty_int_stack(&ds->nodes_to_delete)) {
    *total_deleted = 0;
    return SUCCESS_ECODE_K2T;
  }

  int next_amount_of_nodes = input_block->nodes_count - amount_to_delete;

  int new_size_bits = next_amount_of_nodes * 4;
  int new_container_size = CEIL_OF_DIV(new_size_bits, BITS_SIZE(uint32_t));
  uint32_t *next_container = NULL;
  if (new_container_size > 0) {
    next_container = k2tree_alloc_u32array(new_container_size);
  }

  // int next_node_to_delete = pop_int_stack(&ds->nodes_to_delete);

  int left_side = 0;
  int current_deleted = 0;
  uint32_t frontier_traversal_pos = 0;
  int next_node_to_delete = -1;
  while (!empty_int_stack(&ds->nodes_to_delete)) {
    next_node_to_delete = pop_int_stack(&ds->nodes_to_delete);

    int src_left;
    int dst_left;
    int dst_right;
    src_left = left_side;

    if (left_side == 0) {
      dst_left = 0;
    } else {
      dst_left = left_side - (current_deleted - 1);
    }
    dst_right = next_node_to_delete - (current_deleted + 1);

    int amount = dst_right - dst_left + 1;

    if (amount > 0) {
      int err = (copy_nodes_between_blocks_uarr(
          input_block->container, input_block->container_size, next_container,
          new_container_size, src_left, dst_left, amount));

      if (err) {
        fprintf(stderr, "ERRRORRRR\n");
        exit(1);
      }
    }

    // adjust preorders
    int upper_bound_node = 0;
    if (!empty_int_stack(&ds->nodes_to_delete)) {
      upper_bound_node = top_int_stack(&ds->nodes_to_delete);
    } else {
      upper_bound_node = input_block->nodes_count;
    }

    int is_frontier = frontier_check(input_block, next_node_to_delete,
                                     &frontier_traversal_pos);

    if (is_frontier) {
      NODES_BV_T *new_preorders = NULL;
      struct block *new_children = NULL;
      int next_preorders_amount = (int)input_block->children - 1;
      if (next_preorders_amount > 0) {
        new_preorders = k2tree_alloc_preorders(next_preorders_amount);
        new_children = k2tree_alloc_blocks_array(next_preorders_amount);
        int right_hand_preorders_amount =
            (int)input_block->children - (frontier_traversal_pos + 1);

        if (frontier_traversal_pos > 0) {
          memcpy(new_preorders, input_block->preorders,
                 sizeof(NODES_BV_T) * frontier_traversal_pos);
          memcpy(new_children, input_block->children_blocks,
                 sizeof(struct block) * frontier_traversal_pos);
        }
        if (right_hand_preorders_amount > 0) {
          memcpy(new_preorders + frontier_traversal_pos,
                 input_block->preorders + frontier_traversal_pos + 1,
                 sizeof(NODES_BV_T) * right_hand_preorders_amount);
          memcpy(new_children + frontier_traversal_pos,
                 input_block->children_blocks + frontier_traversal_pos + 1,
                 sizeof(struct block) * right_hand_preorders_amount);
        }
      }

      k2tree_free_preorders(input_block->preorders);
      k2tree_free_blocks_array(input_block->children_blocks);
      input_block->preorders = new_preorders;
      input_block->children_blocks = new_children;
      input_block->children = next_preorders_amount;

      // frontier_traversal_pos = 0;
    }

    for (uint32_t node_pos = next_node_to_delete + 1;
         node_pos < (uint32_t)upper_bound_node; node_pos++) {
      is_frontier =
          frontier_check(input_block, node_pos, &frontier_traversal_pos);
      if (is_frontier) {
        input_block->preorders[frontier_traversal_pos] -= (current_deleted + 1);
      }
    }
    // end: adjust preorders

    left_side = next_node_to_delete;
    current_deleted++;
  }

  if (next_node_to_delete < input_block->nodes_count - 1) {
    int amount = input_block->nodes_count - next_node_to_delete - 1;

    int err = (copy_nodes_between_blocks_uarr(
        input_block->container, input_block->container_size, next_container,
        new_container_size, next_node_to_delete + 1,
        next_node_to_delete - (current_deleted - 1), amount));

    if (err) {
      fprintf(stderr, "ERRRORRRR\n");
      exit(1);
    }
  }

  k2tree_free_u32array(input_block->container, input_block->container_size);
  input_block->container = next_container;
  input_block->container_size = new_container_size;
  input_block->nodes_count = next_amount_of_nodes;
  *total_deleted = amount_to_delete;
  return SUCCESS_ECODE_K2T;
}

int merge_blocks(struct block *parent_block, struct block *child_block,
                 uint32_t frontier_node_in_parent) {

  int merged_nodes = parent_block->nodes_count + child_block->nodes_count - 1;
  int merged_nodes_bits_amount = merged_nodes * 4;
  int new_container_size =
      CEIL_OF_DIV(merged_nodes_bits_amount, BITS_SIZE(uint32_t));
  uint32_t *new_container = k2tree_alloc_u32array(new_container_size);

  int err;
  // build merged topology
  err = (copy_nodes_between_blocks_uarr(
      parent_block->container, parent_block->container_size, new_container,
      new_container_size, 0, 0, (int)frontier_node_in_parent));
  if (err) {
    fprintf(stderr, "Error copying nodes 1\n");
    exit(1);
  }

  err = (copy_nodes_between_blocks_uarr(
      child_block->container, child_block->container_size, new_container,
      new_container_size, 0, (int)frontier_node_in_parent,
      child_block->nodes_count));
  if (err) {
    fprintf(stderr, "Error copying nodes 2\n");
    exit(1);
  }

  int rightmost_amount =
      (int)parent_block->nodes_count - (frontier_node_in_parent + 1);
  if (rightmost_amount > 0) {
    err = (copy_nodes_between_blocks_uarr(
        parent_block->container, parent_block->container_size, new_container,
        new_container_size, (int)(frontier_node_in_parent + 1),
        frontier_node_in_parent + child_block->nodes_count, rightmost_amount));
    if (err) {
      fprintf(stderr, "Error copying nodes 3\n");
      exit(1);
    }
  }

  int new_children_size = parent_block->children - 1 + child_block->children;
  NODES_BV_T *new_preorders = NULL;
  struct block *new_children = NULL;
  if (new_children_size > 0) {
    new_preorders = k2tree_alloc_preorders(new_children_size);
    new_children = k2tree_alloc_blocks_array(new_children_size);
  }

  // Find the preorder of the frontier node in parent which will be removed
  int deleting_preorder_position = -1;
  for (int i = 0; i < (int)parent_block->children; i++) {
    NODES_BV_T curr_preorder = parent_block->preorders[i];
    if (curr_preorder == frontier_node_in_parent) {
      deleting_preorder_position = i;
      break;
    }
  }
  // It always has to be found, because otherwise we wouldn't be merging
  // anything
  assert(deleting_preorder_position >= 0);

  int right_most_preorders_count =
      (int)parent_block->children - 1 - deleting_preorder_position;

  // If zero, then it means we will have an empty frontier
  if (new_children_size > 0) {
    /* children blocks */

    memcpy(new_children, parent_block->children_blocks,
           deleting_preorder_position * sizeof(struct block));
    memcpy(new_children + deleting_preorder_position,
           child_block->children_blocks,
           child_block->children * sizeof(struct block));
    memcpy(new_children + deleting_preorder_position + child_block->children,
           parent_block->children_blocks + deleting_preorder_position + 1,
           right_most_preorders_count * sizeof(struct block));
    /* end:  children blocks */
    /* preorders */
    memcpy(new_preorders, parent_block->preorders,
           deleting_preorder_position * sizeof(NODES_BV_T));

    for (int i = deleting_preorder_position, j = 0;
         i < deleting_preorder_position + child_block->children; i++, j++) {
      // offset by the amount of nodes before the cut
      new_preorders[i] = child_block->preorders[j] + frontier_node_in_parent;
    }

    for (int i = deleting_preorder_position + child_block->children, j = 0;
         i < deleting_preorder_position + child_block->children +
                 right_most_preorders_count;
         i++, j++) {
      new_preorders[i] =
          parent_block->preorders[deleting_preorder_position + 1 + j] +
          child_block->nodes_count - 1;
    }
    /* end: preorders */
  }

  int next_nodes_count =
      parent_block->nodes_count - 1 + child_block->nodes_count;

  k2tree_free_u32array(child_block->container, child_block->container_size);
  k2tree_free_preorders(child_block->preorders);
  k2tree_free_blocks_array(child_block->children_blocks);

  k2tree_free_u32array(parent_block->container, parent_block->container_size);
  k2tree_free_preorders(parent_block->preorders);
  k2tree_free_blocks_array(
      parent_block->children_blocks); // child block ceased to exist

  parent_block->preorders = new_preorders;
  parent_block->children = new_children_size;
  parent_block->container = new_container;
  parent_block->container_size = new_container_size;
  parent_block->children_blocks = new_children;
  parent_block->nodes_count = next_nodes_count;

  return SUCCESS_ECODE_K2T;
}

static int delete_from_node_if_needed(struct block *input_block,
                                      struct block *next_block,
                                      struct deletion_state *ds,
                                      int current_node_id, int child_pos,
                                      int did_merge, int *already_not_exists,
                                      int *has_children) {

  int has_bit = FALSE;
  int bit_position = current_node_id * 4 + child_pos;
  CHECK_ERR(bit_read(input_block, bit_position, &has_bit));

  if (!has_bit) {
    *already_not_exists = TRUE;
    return SUCCESS_ECODE_K2T;
  }

  uint32_t node = 0;
  CHECK_ERR(read_node(input_block, current_node_id, &node));
  int amount_of_children = POP_COUNT(node);

  if (amount_of_children == 1) {
    *has_children = FALSE;
    // printf("piling node %d\n", current_node_id);
    push_int_stack(&ds->nodes_to_delete, (int)current_node_id);
  } else {
    *has_children = TRUE;
    // printf("clearing bit 4*%d + %d\n", current_node_id, child_pos);
    CHECK_ERR(bit_clear(input_block, bit_position));
    if (!did_merge && input_block != next_block) {
      // int next_abs_depth = cr->block_depth + cr->resulting_relative_depth;
      CHECK_ERR(bit_clear(next_block, child_pos));
    }
  }

  return SUCCESS_ECODE_K2T;
}

int delete_point_rec(struct block *input_block, struct deletion_state *ds,
                     struct child_result cr, int *already_not_exists,
                     int *has_children) {

  if (cr.is_leaf_result) {
    return SUCCESS_ECODE_K2T;
  }

  uint32_t current_node_id = cr.resulting_node_idx;

  int current_abs_depth = cr.block_depth + cr.resulting_relative_depth;
  int child_pos = get_code_at_morton_code(&ds->mc, current_abs_depth);
  child(input_block, cr.resulting_node_idx, child_pos,
        cr.resulting_relative_depth, &cr, ds->qs, cr.block_depth);
  if (!cr.exists) {
    *already_not_exists = TRUE;
    return SUCCESS_ECODE_K2T;
  }
  CHECK_ERR(delete_point_rec(cr.resulting_block, ds, cr, already_not_exists,
                             has_children));

  if (*already_not_exists)
    return SUCCESS_ECODE_K2T;

  int total_deleted = 0;
  int previous_nodes_amount = cr.resulting_block->nodes_count;
  int next_amount_of_nodes_child = previous_nodes_amount;

  if (input_block != cr.resulting_block) {
    CHECK_ERR(delete_nodes_in_block(cr.resulting_block, ds, &total_deleted));
    next_amount_of_nodes_child = previous_nodes_amount - total_deleted;
    if (next_amount_of_nodes_child == 0) {
      *has_children = FALSE;
      // return delete_frontier_subtree(ds, current_node_id,
      // cr.resulting_block);
    }
  }

  int did_merge = FALSE;
  if (input_block != cr.resulting_block) {
    if (next_amount_of_nodes_child > 0 &&
        next_amount_of_nodes_child + input_block->nodes_count <
            ds->qs->max_nodes_count) {
      CHECK_ERR(merge_blocks(input_block, cr.resulting_block, current_node_id));
      did_merge = TRUE;
      cr.resulting_block = NULL;
    }
  }

  if (*has_children) {
    return SUCCESS_ECODE_K2T;
  }

  return delete_from_node_if_needed(input_block, cr.resulting_block, ds,
                                    current_node_id, child_pos, did_merge,
                                    already_not_exists, has_children);
}

int delete_point(struct block *input_block, unsigned long col,
                 unsigned long row, struct queries_state *qs,
                 int *already_not_exists) {
  *already_not_exists = FALSE;
  struct deletion_state ds;
  ds.qs = qs;
  init_morton_code(&ds.mc, qs->treedepth);
  init_int_stack(&ds.nodes_to_delete, qs->treedepth);
  convert_coordinates_to_morton_code(col, row, qs->treedepth, &ds.mc);

  struct child_result cr;
  clean_child_result(&cr);
  cr.resulting_block = input_block;
  cr.exists = TRUE;
  int has_children = FALSE;
  CHECK_ERR(delete_point_rec(input_block, &ds, cr, already_not_exists,
                             &has_children));

  int total_deleted = 0;
  CHECK_ERR(delete_nodes_in_block(input_block, &ds, &total_deleted));

  free_int_stack(&ds.nodes_to_delete);
  clean_morton_code(&ds.mc);
  return SUCCESS_ECODE_K2T;
}

declare_stack_of_type(lazy_naive_state)
    declare_stack_of_type(lazy_report_band_state_t)

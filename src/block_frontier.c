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
#include <string.h>

#include "vectors.h"

#include "block.h"
#include "block_frontier.h"

#include "definitions.h"

#include "memalloc.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

/* PRIVATE PROTOTYPES */
int find_insertion_point(struct block_frontier *bf, uint32_t preorder);
/* END PRIVATE PROTOTYPES */

int init_block_frontier(struct block_frontier *bf) {
  _SAFE_OP_K2(vector_uint32_t__init_vector(&(bf->frontier)));
  _SAFE_OP_K2(vector_block_ptr_t__init_vector(&(bf->blocks)));
  return SUCCESS_ECODE_K2T;
}

int init_block_frontier_with_capacity(struct block_frontier *bf,
                                      uint32_t capacity) {
  _SAFE_OP_K2(vector_uint32_t__init_vector_with_capacity(&(bf->frontier),
                                                         (long)capacity));
  _SAFE_OP_K2(
      vector_block_ptr_t__init_vector_with_capacity(&(bf->blocks), capacity));
  return SUCCESS_ECODE_K2T;
}

int free_block_frontier(struct block_frontier *bf) {
  vector_uint32_t__free_vector(&(bf->frontier));
  vector_block_ptr_t__free_vector(&(bf->blocks));
  return SUCCESS_ECODE_K2T;
}

int frontier_check(struct block_frontier *bf, uint32_t node_idx,
                   uint32_t *frontier_traversal_idx, int *result) {
  if (*frontier_traversal_idx >= (uint32_t)bf->frontier.nof_items ||
      bf->frontier.nof_items == 0) {
    *result = FALSE;
    return SUCCESS_ECODE_K2T;
  }

  uint32_t current = bf->frontier.data[*frontier_traversal_idx];

  while (*frontier_traversal_idx < (uint32_t)bf->frontier.nof_items &&
         (current = bf->frontier.data[*frontier_traversal_idx]) < node_idx) {
    (*frontier_traversal_idx)++;
  }

  *result = node_idx == current;

  return SUCCESS_ECODE_K2T;
}

int get_child_block(struct block_frontier *bf, uint32_t frontier_node_idx,
                    struct block **child_block_result) {
  if (frontier_node_idx > (uint32_t)bf->blocks.nof_items) {
    return FRONTIER_OUT_OF_BOUNDS;
  }

  *child_block_result = bf->blocks.data[frontier_node_idx];

  return SUCCESS_ECODE_K2T;
}
int find_insertion_point(struct block_frontier *bf, uint32_t preorder) {
  if (bf->frontier.nof_items == 0) {
    return 0;
  }
  if (bf->frontier.data[0] >= preorder) {
    return 0;
  }

  for (uint32_t i = 0; i < (uint32_t)bf->frontier.nof_items; i++) {
    if (bf->frontier.data[i] > preorder) {
      return i;
    }
  }

  return (int)bf->frontier.nof_items;
}

int extract_sub_block_frontier(struct block_frontier *bf,
                               uint32_t preorder_from, uint32_t preorder_to,
                               struct block_frontier *to_fill_bf) {
  uint32_t from_index_loc =
      find_insertion_point(bf, preorder_from); // inclusive
  if (from_index_loc == (uint32_t)bf->frontier.nof_items) {
    CHECK_ERR(init_block_frontier(to_fill_bf));
    return SUCCESS_ECODE_K2T;
  }

  uint32_t to_index_loc = find_insertion_point(bf, preorder_to); // exclusive
  uint32_t sub_block_size = to_index_loc - from_index_loc;
  uint32_t terminal_block_size = bf->frontier.nof_items - to_index_loc;

  CHECK_ERR(
      init_block_frontier_with_capacity(to_fill_bf, MAX(8, sub_block_size)));

  memcpy(to_fill_bf->frontier.data, bf->frontier.data + from_index_loc,
         sub_block_size * sizeof(uint32_t));
  memcpy(to_fill_bf->blocks.data, bf->blocks.data + from_index_loc,
         sub_block_size * sizeof(block_ptr_t));

  /* shrink data in parent block_frontier -- TODO: evaluate if realloc to
   * smaller container is worthy */
  memmove(bf->frontier.data + from_index_loc, bf->frontier.data + to_index_loc,
          terminal_block_size * sizeof(uint32_t));
  bf->frontier.nof_items = from_index_loc + terminal_block_size;
  memmove(bf->blocks.data + from_index_loc, bf->blocks.data + to_index_loc,
          terminal_block_size * sizeof(block_ptr_t));
  bf->blocks.nof_items = from_index_loc + terminal_block_size;

  return SUCCESS_ECODE_K2T;
}

int add_frontier_node(struct block_frontier *bf,
                      uint32_t new_frontier_node_preorder, struct block *b) {
  uint32_t insertion_point =
      find_insertion_point(bf, new_frontier_node_preorder);
  _SAFE_OP_K2(vector_uint32_t__insert_element_at(
      &(bf->frontier), new_frontier_node_preorder, insertion_point));
  _SAFE_OP_K2(
      vector_block_ptr_t__insert_element_at(&(bf->blocks), b, insertion_point));
  return SUCCESS_ECODE_K2T;
}

struct block_frontier *create_block_frontier(void) {
  struct block_frontier *new_bf = k2tree_alloc_block_frontier();
  init_block_frontier(new_bf);
  return new_bf;
}

/* TODO: (OPTIMIZATION) replace by binary search if needed */
int fix_frontier_indexes(struct block_frontier *bf, uint32_t start, int delta) {
  for (int i = 0; i < bf->frontier.nof_items; i++) {
    uint32_t current_preorder = bf->frontier.data[i];
    if (current_preorder >= start) {
      if ((int)current_preorder < delta) {
        return FIX_INDEXES_PREORDER_HIGHER_THAN_DELTA;
      }
      uint32_t new_val = current_preorder - (uint32_t)delta;
      bf->frontier.data[i] = new_val;
    }
  }
  return SUCCESS_ECODE_K2T;
}

/* TODO: (OPTIMIZATION) replace by binary search if needed to find extreme
 * points */
int collapse_frontier_nodes(struct block_frontier *bf, uint32_t from_preorder,
                            uint32_t to_preorder) {
  /* nothing to do in this case */
  if (bf->frontier.nof_items == 0) {
    return SUCCESS_ECODE_K2T;
  }

  int left_extreme = -1;
  int right_extreme = -1;
  for (int i = 0; i < bf->frontier.nof_items; i++) {
    // uint32_t current_preorder = read_uint_element(&bf->frontier, i);
    uint32_t current_preorder = bf->frontier.data[i];
    if (current_preorder >= from_preorder && left_extreme == -1) {
      left_extreme = i;
    }
    if (current_preorder <= to_preorder) {
      right_extreme = i;
    }
  }
  /* nothing to do in this case */
  if (left_extreme == -1 || right_extreme == -1) {
    return SUCCESS_ECODE_K2T;
  }

  int delete_size = right_extreme - left_extreme + 1;
  int new_size = bf->frontier.nof_items - delete_size;

  memmove(&bf->frontier + left_extreme, &bf->frontier + (right_extreme + 1),
          (bf->frontier.nof_items - right_extreme) * sizeof(uint32_t));

  bf->frontier.nof_items = new_size;

  memmove(&bf->blocks + left_extreme, &bf->blocks + (right_extreme + 1),
          (bf->blocks.nof_items - right_extreme) * sizeof(block_ptr_t));

  bf->blocks.nof_items = new_size;

  return SUCCESS_ECODE_K2T;
}

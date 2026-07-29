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
int find_insertion_point(struct block *input_block, uint32_t preorder);
/* END PRIVATE PROTOTYPES */

void init_block_frontier(struct block *input_block) {
  input_block->preorders = NULL;
  input_block->children_blocks = NULL;
  input_block->children = 0;
}

void init_block_frontier_with_capacity(struct block *input_block,
                                       uint32_t capacity) {

  input_block->preorders = (NODES_BV_T *)malloc(sizeof(NODES_BV_T) * capacity);
  input_block->children_blocks =
      (struct block **)malloc(sizeof(struct block *) * capacity);
}

void free_block_frontier(struct block *input_block) {

  if (input_block->children > 0) {
    free(input_block->preorders);
    free(input_block->children_blocks);
  }
}

int frontier_check(struct block *input_block, uint32_t node_idx,
                   uint32_t *frontier_traversal_idx) {
  uint32_t tmp_findex = *frontier_traversal_idx;
  uint32_t children = (uint32_t)input_block->children;
  if (tmp_findex >= children || input_block->children == 0) {
    return FALSE;
  }

  uint32_t current;

  while (tmp_findex < children &&
         (current = input_block->preorders[tmp_findex]) < node_idx) {
    tmp_findex++;
  }

  *frontier_traversal_idx = tmp_findex;

  return node_idx == current;
}

struct block *get_child_block(struct block *input_block,
                              uint32_t frontier_node_idx) {
  if (frontier_node_idx > input_block->children) {
    fprintf(stderr, "frontier node out of bound %d > %d",
            (int)frontier_node_idx, (int)input_block->children);
    exit(1);
  }
  return input_block->children_blocks[frontier_node_idx];
}

int find_insertion_point(struct block *input_block, uint32_t preorder) {
  if (input_block->children == 0) {
    return 0;
  }
  if (input_block->preorders[0] >= preorder) {
    return 0;
  }

  for (NODES_BV_T i = 0; i < input_block->children; i++) {
    if (input_block->preorders[i] > preorder) {
      return i;
    }
  }

  return (int)input_block->children;
}

int extract_sub_block_frontier(struct block *input_block,
                               uint32_t preorder_from, uint32_t preorder_to,
                               struct block *to_fill_bf) {
  uint32_t from_index_loc =
      find_insertion_point(input_block, preorder_from); // inclusive
  if (from_index_loc == (uint32_t)input_block->children) {
    init_block_frontier(to_fill_bf);
    return SUCCESS_ECODE_K2T;
  }

  uint32_t to_index_loc =
      find_insertion_point(input_block, preorder_to); // exclusive
  uint32_t sub_block_size = to_index_loc - from_index_loc;
  uint32_t terminal_block_size = input_block->children - to_index_loc;

  free_block_frontier(to_fill_bf);

  if (sub_block_size > 0)
    init_block_frontier_with_capacity(to_fill_bf, sub_block_size);

  if (sub_block_size > 0) {

    memcpy(to_fill_bf->preorders, input_block->preorders + from_index_loc,
           sub_block_size * sizeof(uint32_t));
    memcpy(to_fill_bf->children_blocks,
           input_block->children_blocks + from_index_loc,
           sub_block_size * sizeof(struct block *));
  }

  uint32_t next_children = from_index_loc + terminal_block_size;
  NODES_BV_T *new_parent_preorders = NULL;
  struct block **new_parent_cblocks = NULL;
  if (next_children > 0) {
    new_parent_preorders =
        (NODES_BV_T *)malloc(sizeof(NODES_BV_T) * next_children);
    new_parent_cblocks =
        (struct block **)malloc(sizeof(struct block *) * next_children);
    memcpy(new_parent_preorders, input_block->preorders,
           from_index_loc * sizeof(NODES_BV_T));
    memcpy(new_parent_cblocks, input_block->children_blocks,
           from_index_loc * sizeof(struct block *));
    memcpy(new_parent_preorders + from_index_loc,
           input_block->preorders + to_index_loc,
           terminal_block_size * sizeof(NODES_BV_T));
    memcpy(new_parent_cblocks + from_index_loc,
           input_block->children_blocks + to_index_loc,
           terminal_block_size * sizeof(struct block *));
  }

  free_block_frontier(input_block);

  input_block->preorders = new_parent_preorders;
  input_block->children_blocks = new_parent_cblocks;
  input_block->children = next_children;

  return SUCCESS_ECODE_K2T;
}

int add_frontier_node(struct block *input_block,
                      uint32_t new_frontier_node_preorder, struct block *b) {
  uint32_t insertion_point =
      find_insertion_point(input_block, new_frontier_node_preorder);

  uint32_t children = (uint32_t)input_block->children;
  NODES_BV_T *new_preorders =
      (NODES_BV_T *)malloc(sizeof(NODES_BV_T) * (children + 1));
  struct block **new_children_blocks =
      (struct block **)malloc(sizeof(struct block *) * (children + 1));
  if (children > 0) {
    memcpy(new_preorders, input_block->preorders,
           insertion_point * sizeof(NODES_BV_T));
    memcpy(new_children_blocks, input_block->children_blocks,
           insertion_point * sizeof(struct block *));
  }

  if (children > insertion_point) {
    memcpy(new_preorders + insertion_point + 1,
           input_block->preorders + insertion_point,
           (children - insertion_point) * sizeof(NODES_BV_T));
    memcpy(new_children_blocks + insertion_point + 1,
           input_block->children_blocks + insertion_point,
           (children - insertion_point) * sizeof(struct block *));
  }

  new_preorders[insertion_point] = new_frontier_node_preorder;
  new_children_blocks[insertion_point] = b;

  free_block_frontier(input_block);

  input_block->preorders = new_preorders;
  input_block->children_blocks = new_children_blocks;
  input_block->children++;

  return SUCCESS_ECODE_K2T;
}

/* TODO: (OPTIMIZATION) replace by binary search if needed */
int fix_frontier_indexes(struct block *input_block, uint32_t start, int delta) {
  for (int i = 0; i < input_block->children; i++) {
    uint32_t current_preorder = input_block->preorders[i];
    if (current_preorder >= start) {
      if ((int)current_preorder < delta) {
        return FIX_INDEXES_PREORDER_HIGHER_THAN_DELTA;
      }
      uint32_t new_val = current_preorder - (uint32_t)delta;
      input_block->preorders[i] = (NODES_BV_T)new_val;
    }
  }
  return SUCCESS_ECODE_K2T;
}

/* TODO: (OPTIMIZATION) replace by binary search if needed to find extreme
 * points */
int collapse_frontier_nodes(struct block *input_block, uint32_t from_preorder,
                            uint32_t to_preorder) {
  /* nothing to do in this case */
  if (input_block->children == 0) {
    return SUCCESS_ECODE_K2T;
  }

  int left_extreme = -1;
  int right_extreme = -1;
  for (int i = 0; i < input_block->children; i++) {
    // uint32_t current_preorder = read_uint_element(&bf->frontier, i);
    uint32_t current_preorder = input_block->preorders[i];
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
  int new_size = input_block->children - delete_size;

  uint32_t children = (uint32_t)input_block->children;

  memmove(input_block->preorders + left_extreme,
          input_block->preorders + (right_extreme + 1),
          (children - right_extreme) * sizeof(uint32_t));

  memmove(input_block->children_blocks + left_extreme,
          input_block->children_blocks + (right_extreme + 1),
          (children - right_extreme) * sizeof(struct block *));

  input_block->children = new_size;

  return SUCCESS_ECODE_K2T;
}

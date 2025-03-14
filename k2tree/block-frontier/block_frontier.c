#include <string.h>

#include <vector.h>

#include "block/block.h"
#include "block_frontier.h"

#include "definitions.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

/* PRIVATE PROTOTYPES */
int find_insertion_point(struct block_frontier *bf, uint32_t preorder);
/* END PRIVATE PROTOTYPES */

int init_block_frontier(struct block_frontier *bf) {
  init_vector(&(bf->frontier), sizeof(uint32_t));
  init_vector(&(bf->blocks), sizeof(struct block *));
  return SUCCESS_ECODE;
}

int init_block_frontier_with_capacity(struct block_frontier *bf,
                                      uint32_t capacity) {
  init_vector_with_capacity(&(bf->frontier), sizeof(uint32_t), capacity);
  init_vector_with_capacity(&(bf->blocks), sizeof(struct block *), capacity);
  return SUCCESS_ECODE;
}

int free_block_frontier(struct block_frontier *bf) {
  free_vector(&(bf->frontier));
  free_vector(&(bf->blocks));
  return SUCCESS_ECODE;
}

int frontier_check(struct block_frontier *bf, uint32_t node_idx,
                   uint32_t *frontier_traversal_idx, int *result) {
  if (*frontier_traversal_idx >= (uint32_t)bf->frontier.nof_items ||
      bf->frontier.nof_items == 0) {
    *result = FALSE;
    return SUCCESS_ECODE;
  }

  uint32_t current = read_uint_element(&bf->frontier, *frontier_traversal_idx);

  while (*frontier_traversal_idx < (uint32_t)bf->frontier.nof_items &&
         (current = read_uint_element(&bf->frontier, *frontier_traversal_idx)) <
             node_idx) {
    (*frontier_traversal_idx)++;
  }

  *result = node_idx == current;

  return SUCCESS_ECODE;
}

int get_child_block(struct block_frontier *bf, uint32_t frontier_node_idx,
                    struct block **child_block_result) {
  if (frontier_node_idx > (uint32_t)bf->blocks.nof_items) {
    return FRONTIER_OUT_OF_BOUNDS;
  }

  *child_block_result = read_block_element(&bf->blocks, frontier_node_idx);

  return SUCCESS_ECODE;
}
int find_insertion_point(struct block_frontier *bf, uint32_t preorder) {
  if (bf->frontier.nof_items == 0) {
    return 0;
  }
  if (read_uint_element(&bf->frontier, 0) >= preorder) {
    return 0;
  }

  for (uint32_t i = 0; i < (uint32_t)bf->frontier.nof_items; i++) {
    if (read_uint_element(&bf->frontier, i) > preorder) {
      return i;
    }
  }

  return bf->frontier.nof_items;
}
int extract_sub_block_frontier(struct block_frontier *bf,
                               uint32_t preorder_from, uint32_t preorder_to,
                               struct block_frontier *to_fill_bf) {
  uint32_t from_index_loc =
      find_insertion_point(bf, preorder_from); // inclusive
  if (from_index_loc == (uint32_t)bf->frontier.nof_items) {
    _SAFE_OP_K2(init_block_frontier(to_fill_bf));
    return SUCCESS_ECODE;
  }

  uint32_t to_index_loc = find_insertion_point(bf, preorder_to); // exclusive
  uint32_t sub_block_size = to_index_loc - from_index_loc;
  uint32_t terminal_block_size = bf->frontier.nof_items - to_index_loc;

  _SAFE_OP_K2(
      init_block_frontier_with_capacity(to_fill_bf, MAX(8, sub_block_size)));

  memcpy(to_fill_bf->frontier.data,
         bf->frontier.data + from_index_loc * bf->frontier.element_size,
         sub_block_size * bf->frontier.element_size);
  memcpy(to_fill_bf->blocks.data,
         bf->blocks.data + from_index_loc * bf->blocks.element_size,
         sub_block_size * bf->blocks.element_size);

  /* shrink data in parent block_frontier -- TODO: evaluate if realloc to
   * smaller container is worthy */
  memmove(bf->frontier.data + from_index_loc * bf->frontier.element_size,
          bf->frontier.data + to_index_loc * bf->frontier.element_size,
          terminal_block_size * bf->frontier.element_size);
  memmove(bf->blocks.data + from_index_loc * bf->blocks.element_size,
          bf->blocks.data + to_index_loc * bf->blocks.element_size,
          terminal_block_size * bf->blocks.element_size);

  return SUCCESS_ECODE;
}

int add_frontier_node(struct block_frontier *bf,
                      uint32_t new_frontier_node_preorder, struct block *b) {
  uint32_t insertion_point =
      find_insertion_point(bf, new_frontier_node_preorder);
  _SAFE_OP_K2(insert_element_at(
      &bf->frontier, (char *)&new_frontier_node_preorder, insertion_point));
  _SAFE_OP_K2(insert_element_at(&bf->blocks, (char *)&b, insertion_point));
  return SUCCESS_ECODE;
}

struct block_frontier *create_block_frontier(void) {
  struct block_frontier *new_bf = calloc(1, sizeof(struct block_frontier));
  init_block_frontier(new_bf);
  return new_bf;
}

/* TODO: (OPTIMIZATION) replace by binary search if needed */
int fix_frontier_indexes(struct block_frontier *bf, uint32_t start, int delta) {
  for (int i = 0; i < bf->frontier.nof_items; i++) {
    uint32_t current_preorder = read_uint_element(&bf->frontier, i);
    if (current_preorder >= start) {
      if ((int)current_preorder < delta) {
        return FIX_INDEXES_PREORDER_HIGHER_THAN_DELTA;
      }
      uint32_t new_val = current_preorder - (uint32_t)delta;
      _SAFE_OP_K2(set_element_at(&bf->frontier, (char *)&new_val, i));
    }
  }
  return SUCCESS_ECODE;
}

/* TODO: (OPTIMIZATION) replace by binary search if needed to find extreme
 * points */
int collapse_frontier_nodes(struct block_frontier *bf, uint32_t from_preorder,
                            uint32_t to_preorder) {
  /* nothing to do in this case */
  if (bf->frontier.nof_items == 0) {
    return SUCCESS_ECODE;
  }

  int left_extreme = -1;
  int right_extreme = -1;
  for (int i = 0; i < bf->frontier.nof_items; i++) {
    uint32_t current_preorder = read_uint_element(&bf->frontier, i);
    if (current_preorder >= from_preorder && left_extreme == -1) {
      left_extreme = i;
    }
    if (current_preorder <= to_preorder) {
      right_extreme = i;
    }
  }
  /* nothing to do in this case */
  if (left_extreme == -1 || right_extreme == -1) {
    return SUCCESS_ECODE;
  }

  int delete_size = right_extreme - left_extreme + 1;
  int new_size = bf->frontier.nof_items - delete_size;

  memmove(&bf->frontier + left_extreme * bf->frontier.element_size,
          &bf->frontier + (right_extreme + 1) * bf->frontier.element_size,
          (bf->frontier.nof_items - right_extreme) * bf->frontier.element_size);

  bf->frontier.nof_items = new_size;

  memmove(&bf->blocks + left_extreme * bf->blocks.element_size,
          &bf->blocks + (right_extreme + 1) * bf->blocks.element_size,
          (bf->blocks.nof_items - right_extreme) * bf->blocks.element_size);

  bf->blocks.nof_items = new_size;

  return SUCCESS_ECODE;
}

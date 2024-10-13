#include "block_frontier.h"
#include "block/block.h"

#include "definitions.h"

#include <vector.h>


int init_block_frontier(struct block_frontier *bf) {
  init_vector(&(bf->frontier), sizeof(uint32_t));
  init_vector(&(bf->blocks), sizeof(struct block *));
}

int free_block_frontier(struct block_frontier *bf) {
  free_vector(&(bf->frontier));
  free_vector(&(bf->blocks));
}

int frontier_check(struct block_frontier *bf, uint32_t node_idx,
                   uint32_t *frontier_traversal_idx, int *result) {
  if (*frontier_traversal_idx >= bf->frontier.nof_items ||
      bf->frontier.nof_items == 0) {
    *result = FALSE;
    return SUCCESS_ECODE;
  }

  uint32_t current = read_uint_element(&bf->frontier, *frontier_traversal_idx);

  while (*frontier_traversal_idx < bf->frontier.nof_items &&
         (current = read_uint_element(&bf->frontier, *frontier_traversal_idx)) <
             node_idx) {
    *frontier_traversal_idx++;
  }

  *result = node_idx == current;

  return SUCCESS_ECODE;
}

int get_child_block(struct block_frontier *bf, uint32_t frontier_node_idx, struct block **child_block_result){
    if(frontier_node_idx > bf->blocks.nof_items){
        return FRONTIER_OUT_OF_BOUNDS;
    }

    *child_block_result = read_block_element(&bf->blocks, frontier_node_idx);
    
    return SUCCESS_ECODE;
}
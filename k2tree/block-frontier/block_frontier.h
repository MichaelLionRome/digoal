#ifndef _BLOCK_FRONTIER_H
#define _BLOCK_FRONTIER_H

#include <vector.h>

#include "block/block.h"
#include "definitions.h"

struct block_frontier {
  struct vector frontier;
  struct vector blocks;
};

int init_block_frontier(struct block_frontier *bf);
int init_block_frontier_with_capacity(struct block_frontier *bf, int capacity);
int free_block_frontier(struct block_frontier *bf);
int frontier_check(struct block_frontier *bf, uint32_t node_idx,
                   uint32_t *frontier_traversal_idx, int *result);

int get_child_block(struct block_frontier *bf, uint32_t frontier_node_idx,
                    struct block **child_block_result);

int extract_sub_block_frontier(struct block_frontier *bf,
                               uint32_t preorder_from, uint32_t preorder_to,
                               struct block_frontier *to_fill_bf);

int add_frontier_node(struct block_frontier *bf,
                      uint32_t new_frontier_node_preorder, struct block *b);

#endif /* _BLOCK_FRONTIER_H */

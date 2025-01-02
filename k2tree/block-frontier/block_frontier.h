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
int free_block_frontier(struct block_frontier *bf);
int frontier_check(struct block_frontier *bf, uint32_t node_idx,
                   uint32_t *frontier_traversal_idx, int *result);

int get_child_block(struct block_frontier *bf, uint32_t frontier_node_idx,
                    struct block **child_block_result);

#endif /* _BLOCK_FRONTIER_H */

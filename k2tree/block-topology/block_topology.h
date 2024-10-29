#ifndef _BLOCK_TOPOLOGY_H
#define _BLOCK_TOPOLOGY_H

#include <bitvector.h>
#include <stdint.h>

#include "definitions.h"

struct block_topology {
  struct bitvector *bv;
  uint32_t nodes_count;
};

int init_block_topology(struct block_topology *bt, struct bitvector *bv,
                        uint32_t nodes_count);

int child_exists(struct block_topology *bt, uint32_t input_node_idx,
                 uint32_t requested_child_position, int *result);

int read_node(struct block_topology *bt, uint32_t node_idx, uint32_t *result);

int count_children(struct block_topology *bt, uint32_t node_idx,
                   uint32_t *result);

#endif /* _BLOCK_TOPOLOGY_H */

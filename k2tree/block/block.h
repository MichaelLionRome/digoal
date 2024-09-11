#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <stdint.h>

#include "block-topology/block_topology.h"

struct block {
  struct block_topology *bt;
  struct block_frontier *bf;
  uint32_t block_depth;
  uint32_t tree_depth;
};

int has_point(struct block *input_block, uint32_t col, uint32_t row);
int insert_point(struct block *input_block, uint32_t col, uint32_t row);

#endif /* _BLOCK_H_ */

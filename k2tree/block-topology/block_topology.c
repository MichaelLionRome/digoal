#include "block_topology.h"

int init_block_topology(struct block_topology *bt, struct bitvector *bv,
                        uint32_t nodes_count) {
  bt->bv = bv;
  bt->nodes_count = nodes_count;
  return 0;
}

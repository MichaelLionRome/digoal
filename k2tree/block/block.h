#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <stdint.h>

#include "block-topology/block_topology.h"

typedef unsigned long ulong;
struct block {
  struct block_topology *bt;
  struct block_frontier *bf;
  uint32_t block_depth;
  uint32_t tree_depth;
};

int has_point(struct block *input_block, ulong col, ulong row,
              struct morton_code *mc, struct sequential_scan_result *sc_result,
              struct circular_queue *not_yet_traversed,
              struct circular_queue *subtrees_count, int *result);
int insert_point(struct block *input_block, ulong col, ulong row,
                 struct morton_code *mc,
                 struct sequential_scan_result *sc_result,
                 struct circular_queue *not_yet_traversed,
                 struct circular_queue *subtrees_count);

#endif /* _BLOCK_H_ */

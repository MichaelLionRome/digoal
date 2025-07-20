#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <stdint.h>

#include <vector.h>

#include "block-frontier/block_frontier.h"
#include "block-topology/block_topology.h"

#include "queries_state.h"

struct block {
  struct block_topology *bt;
  struct block_frontier *bf;
  uint32_t block_depth;
  uint32_t tree_depth;
  uint32_t max_node_count;

  struct block *root;
};

int has_point(struct block *input_block, ulong col, ulong row,
              struct queries_state *qs, int *result);
int insert_point(struct block *input_block, ulong col, ulong row,
                 struct queries_state *qs);

int naive_scan_points(struct block *input_block, struct queries_state *qs,
                      struct vector *result);

struct block *create_block(uint32_t tree_depth);
int free_rec_block(struct block *input_block);
int free_block(struct block *input_block);

#endif /* _BLOCK_H_ */

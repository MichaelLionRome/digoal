#include "block_topology.h"

int init_block_topology(struct block_topology *bt, struct bitvector *bv,
                        uint32_t nodes_count) {
  bt->bv = bv;
  bt->nodes_count = nodes_count;
  return 0;
}

int child_exists(struct block_topology *bt, uint32_t input_node_idx,
                 uint32_t requested_child_position, int *result) {
  struct bitvector *bv = bt->bv;
  int bit_on;
  CHECK_ERR(
      bit_read(bv, input_node_idx * 4 + requested_child_position, &bit_on));
  *result = bit_on;

  return SUCCESS_ECODE;
}

int read_node(struct block_topology *bt, uint32_t node_idx, uint32_t *result) {
  CHECK_ERR(bits_read(bt->bv, 4 * node_idx, 4 * (node_idx + 1) - 1, result));
  return SUCCESS_ECODE;
}

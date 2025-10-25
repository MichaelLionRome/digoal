#ifndef _BLOCK_TOPOLOGY_H
#define _BLOCK_TOPOLOGY_H

#include <bitvector.h>
#include <stdint.h>

typedef uint16_t NODES_COUNT_T;
struct block_topology {
  struct bitvector *bv;
  NODES_COUNT_T nodes_count;
};

struct block_topology *create_block_topology(void);

int free_block_topology(struct block_topology *bt);

int init_block_topology(struct block_topology *bt, struct bitvector *bv,
                        NODES_COUNT_T nodes_count);
int child_exists(struct block_topology *bt, NODES_COUNT_T input_node_idx,
                 uint32_t requested_child_position, int *result);
int read_node(struct block_topology *bt, NODES_COUNT_T node_idx,
              uint32_t *result);
int count_children(struct block_topology *bt, NODES_COUNT_T node_idx,
                   uint32_t *result);
int enlarge_block_size_to(struct block_topology *bt, uint32_t new_block_size);
int shift_right_nodes_after(struct block_topology *bt, NODES_COUNT_T node_index,
                            NODES_COUNT_T nodes_to_insert);
NODES_COUNT_T get_allocated_nodes(struct block_topology *bt);
int mark_child_in_node(struct block_topology *bt, NODES_COUNT_T node_index,
                       uint32_t leaf_child);
int insert_node_at(struct block_topology *bt, NODES_COUNT_T node_index,
                   uint32_t code);
int extract_sub_bitvector(struct block_topology *bt, uint32_t from, uint32_t to,
                          struct bitvector *result);
int collapse_nodes(struct block_topology *bt, uint32_t from, uint32_t to);

#endif /* _BLOCK_TOPOLOGY_H */

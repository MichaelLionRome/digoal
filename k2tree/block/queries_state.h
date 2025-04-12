#ifndef _QUERIES_STATE_H
#define _QUERIES_STATE_H

#include <bitvector.h>
#include <circular_queue.h>
#include <vector.h>

#include "morton-code/morton_code.h"

struct sequential_scan_result {
  uint32_t child_preorder;
  uint32_t node_relative_depth;
  struct vector *subtrees_count_map;
  struct vector *relative_depth_map;
};

struct node_subtree_info {
  uint32_t node_index;
  uint32_t node_relative_depth;
  uint32_t subtree_size;
};
struct queries_state {
  struct morton_code mc;
  struct sequential_scan_result sc_result;
  struct circular_queue not_yet_traversed;
  struct circular_queue subtrees_count;
  int find_split_data;
};

int init_queries_state(struct queries_state *qs, uint32_t tree_depth);
int finish_queries_state(struct queries_state *qs);

#endif /* _QUERIES_STATE_H */

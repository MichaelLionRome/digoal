#ifndef _QUERIES_STATE_H
#define _QUERIES_STATE_H

#include <bitvector.h>
#include <circular_queue.h>
#include <vector.h>

#include "block/block.h"
#include "morton-code/morton_code.h"

struct sequential_scan_result {
  uint32_t child_preorder;
  uint32_t node_relative_depth;
  struct vector *subtrees_count_map;
  struct vector *relative_depth_map;
};

struct queries_state {
  struct morton_code mc;
  struct sequential_scan_result sc_result;
  struct circular_queue not_yet_traversed;
  struct circular_queue subtrees_count;
};

#endif /* _QUERIES_STATE_H */

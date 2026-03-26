/*
MIT License

Copyright (c) 2020 Cristobal Miranda T.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef _QUERIES_STATE_H
#define _QUERIES_STATE_H

#include <bitvector.h>

#include "definitions.h"
#include "memalloc.h"
#include "morton_code.h"
#include "stacks.h"

struct sequential_scan_result {
  uint32_t child_preorder;
  uint32_t node_relative_depth;
  struct u32array_alloc subtrees_count_map;
  struct u32array_alloc relative_depth_map;
};

#ifdef DEBUG_STATS
struct debug_stats {
  unsigned long time_on_sequential_scan;
  unsigned long time_on_frontier_check;
  unsigned long split_count;
};
#endif

struct queries_state {
  struct morton_code mc;
  struct sequential_scan_result sc_result;
  struct int_stack not_yet_traversed;
  struct nsi_t_stack subtrees_count;
  int find_split_data;
#ifdef DEBUG_STATS
  struct debug_stats dstats;
#endif
};

int init_queries_state(struct queries_state *qs, uint32_t tree_depth);
int finish_queries_state(struct queries_state *qs);

#endif /* _QUERIES_STATE_H */

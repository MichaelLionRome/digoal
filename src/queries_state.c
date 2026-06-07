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
#include "queries_state.h"

/* PRIVATE PROTOTYPES */
int init_sequential_scan_result(struct sequential_scan_result *scr,
                                MAX_NODE_COUNT_T max_nodes_count);
int clean_sequential_scan_result(struct sequential_scan_result *scr);
/* END PRIVATE PROTOTYPES */

/* IMPLEMENTATION PUBLIC FUNCTIONS */
int init_queries_state(struct queries_state *qs, uint32_t tree_depth,
                       MAX_NODE_COUNT_T max_nodes_count,
                       struct block *root_block) {
  init_morton_code(&(qs->mc), tree_depth);
  init_int_stack(&(qs->not_yet_traversed), 2 * tree_depth);
  init_nsi_t_stack(&(qs->subtrees_count), 2 * tree_depth);
  qs->find_split_data = FALSE;
  qs->max_nodes_count = max_nodes_count;
  qs->root = root_block;
  qs->treedepth = tree_depth;
#ifdef DEBUG_STATS
  qs->dstats.time_on_sequential_scan = 0;
  qs->dstats.time_on_frontier_check = 0;
  qs->dstats.split_count = 0;
#endif
  return init_sequential_scan_result(&qs->sc_result, qs->max_nodes_count);
}

int finish_queries_state(struct queries_state *qs) {
  free_nsi_t_stack(&qs->subtrees_count);
  free_int_stack(&qs->not_yet_traversed);
  clean_morton_code(&qs->mc);
  return clean_sequential_scan_result(&qs->sc_result);
}
/* END IMPLEMENTATION PUBLIC FUNCTIONS */

/* PRIVATE FUNCTIONS IMPLEMENTATION */
int init_sequential_scan_result(struct sequential_scan_result *scr,
                                MAX_NODE_COUNT_T max_nodes_count) {
  scr->subtrees_count_map = k2tree_alloc_u32array(max_nodes_count);
  scr->relative_depth_map = k2tree_alloc_u32array(max_nodes_count);

  return SUCCESS_ECODE_K2T;
}

int clean_sequential_scan_result(struct sequential_scan_result *scr) {

  k2tree_free_u32array(scr->subtrees_count_map);
  k2tree_free_u32array(scr->relative_depth_map);

  return SUCCESS_ECODE_K2T;
}
/* END PRIVATE FUNCTIONS IMPLEMENTATION */

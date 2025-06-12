#include "queries_state.h"
#include "definitions.h"

/* PRIVATE PROTOTYPES */
int init_sequential_scan_result(struct sequential_scan_result *scr);
int clean_sequential_scan_result(struct sequential_scan_result *scr);
/* END PRIVATE PROTOTYPES */

/* IMPLEMENTATION PUBLIC FUNCTIONS */
int init_queries_state(struct queries_state *qs, uint32_t tree_depth) {
  CHECK_ERR(init_morton_code(&qs->mc, tree_depth));
  init_circular_queue(&qs->not_yet_traversed, 2 * tree_depth, sizeof(uint32_t));
  init_circular_queue(&qs->subtrees_count, 2 * tree_depth,
                      sizeof(struct node_subtree_info));
  qs->find_split_data = FALSE;
  return init_sequential_scan_result(&qs->sc_result);
}

int finish_queries_state(struct queries_state *qs) {
  _SAFE_OP_K2(clean_circular_queue(&qs->subtrees_count));
  _SAFE_OP_K2(clean_circular_queue(&qs->not_yet_traversed));
  CHECK_ERR(clean_morton_code(&qs->mc));
  return clean_sequential_scan_result(&qs->sc_result);
}
/* END IMPLEMENTATION PUBLIC FUNCTIONS */

/* PRIVATE FUNCTIONS IMPLEMENTATION */
int init_sequential_scan_result(struct sequential_scan_result *scr) {
  scr->subtrees_count_map = (struct vector *)calloc(1, sizeof(struct vector));
  scr->relative_depth_map = (struct vector *)calloc(1, sizeof(struct vector));
  _SAFE_OP_K2(init_vector_with_capacity(scr->subtrees_count_map,
                                        sizeof(uint32_t), MAX_NODES_IN_BLOCK));
  _SAFE_OP_K2(init_vector_with_capacity(scr->relative_depth_map,
                                        sizeof(uint32_t), MAX_NODES_IN_BLOCK));
  return SUCCESS_ECODE;
}

int clean_sequential_scan_result(struct sequential_scan_result *scr) {
  _SAFE_OP_K2(free_vector(scr->subtrees_count_map));
  _SAFE_OP_K2(free_vector(scr->relative_depth_map));
  free(scr->subtrees_count_map);
  free(scr->relative_depth_map);
  return SUCCESS_ECODE;
}
/* END PRIVATE FUNCTIONS IMPLEMENTATION */

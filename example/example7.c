#include <definitions.h>
#include <k2node.h>
#include <queries_state.h>
#include <stdio.h>
#include <vectors.h>

int main(void) {

  struct k2node *root_node = create_k2node();

  TREE_DEPTH_T treedepth = 5;
  TREE_DEPTH_T cut_depth = 2;
  TREE_DEPTH_T subtree_depth = treedepth - cut_depth;

  struct queries_state qs;
  init_queries_state(&qs, subtree_depth, 256, NULL);

  struct k2qstate st;
  st.cut_depth = cut_depth;
  st.k2tree_depth = treedepth;
  st.qs = &qs;

  ulong side = 1 << treedepth;
  for (ulong col = 0; col < side; col++) {
    for (ulong row = 0; row < side; row++) {
      k2node_insert_point(root_node, col, row, &st);
    }
  }

  int has_the_point;
  k2node_has_point(root_node, 30, 30, &st, &has_the_point);

  printf("Has point (30, 30)?: ");
  if (has_the_point) {
    printf(" YES!\n");
  } else {
    printf(" NO\n");
  }

  struct vector_pair2dl_t result;

  vector_pair2dl_t__init_vector(&result);

  k2node_naive_scan_points(root_node, &st, &result);

  for (int i = 0; i < result.nof_items; i++) {
    printf("(%lu, %lu) ", result.data[i].col, result.data[i].row);
  }
  printf("\n");

  free_rec_k2node(root_node, 0, st.cut_depth);
  finish_queries_state(&qs);
}
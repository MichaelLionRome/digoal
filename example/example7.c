#include <definitions.h>
#include <k2node.h>
#include <queries_state.h>
#include <stdio.h>
#include <vectors.h>

int sort_fun(const void *lhs, const void *rhs);

int main(void) {

  struct k2node *root_node = create_k2node();

  TREE_DEPTH_T treedepth = 4;
  TREE_DEPTH_T cut_depth = 2;

  struct k2qstate st;
  init_k2qstate(&st, treedepth, 255, cut_depth);

  ulong side = 1 << treedepth;
  for (ulong col = 0; col < side; col++) {
    for (ulong row = 0; row < side; row++) {
      k2node_insert_point(root_node, col, row, &st);
    }
  }

  int has_the_point;
  k2node_has_point(root_node, 6, 0, &st, &has_the_point);

  printf("Has point (6, 1)?: ");
  if (has_the_point) {
    printf(" YES!\n");
  } else {
    printf(" NO\n");
  }

  struct vector_pair2dl_t result;

  vector_pair2dl_t__init_vector(&result);

  k2node_naive_scan_points(root_node, &st, &result);

  qsort(result.data, result.nof_items, sizeof(struct pair2dl), sort_fun);

  for (int i = 0; i < result.nof_items; i++) {
    if (i > 0 && result.data[i].col > result.data[i - 1].col)
      printf("\n");
    printf("(%lu, %lu) ", result.data[i].col, result.data[i].row);
  }
  printf("\n");

  free_rec_k2node(root_node, 0, st.cut_depth);
  clean_k2qstate(&st);
}

int sort_fun(const void *lhs, const void *rhs) {
  struct pair2dl lhs_pair = *(struct pair2dl *)lhs;
  struct pair2dl rhs_pair = *(struct pair2dl *)rhs;
  return (lhs_pair.col != rhs_pair.col
              ? (long)lhs_pair.col - (long)rhs_pair.col
              : (long)lhs_pair.row - (long)rhs_pair.row);
}

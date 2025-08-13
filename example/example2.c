#include <block.h>
#include <queries_state.h>

#include <stdio.h>

#include "definitions.h"

int main(void) {
  int err_code;
  uint32_t treedepth = 25;
  struct block *root_block = create_block(treedepth);

  struct queries_state qs;
  init_queries_state(&qs, treedepth);

  struct pair2dl array_pairs[] = {{0, 0},  {3, 3},   {15, 3},
                                  {3, 15}, {30, 31}, {31, 8}};

  // struct pair2dl array_pairs[] = {{0, 0}, {3, 3}};
  int qty = sizeof(array_pairs) / sizeof(struct pair2dl);
  for (int i = 0; i < qty; i++) {
    printf("inserting %lu, %lu\n", array_pairs[i].col, array_pairs[i].row);
    insert_point(root_block, array_pairs[i].col, array_pairs[i].row, &qs);
    int does_have_point;
    has_point(root_block, array_pairs[i].col, array_pairs[i].row, &qs,
              &does_have_point);
    if (!does_have_point) {
      printf("doesn't have point (%lu, %lu)\n", array_pairs[i].col,
             array_pairs[i].row);
      goto clean_up;
    }
  }

  struct vector result;
  init_vector_with_capacity(&result, sizeof(struct pair2dl), qty);

  err_code = naive_scan_points(root_block, &qs, &result);
  if (err_code) {
    exit(err_code);
  }

  printf("scanned points:\n");
  for (int i = 0; i < result.nof_items; i++) {
    struct pair2dl *current;
    get_element_at(&result, i, (char **)&current);
    printf("element %d = (%lu, %lu)\n", i, current->col, current->row);
  }

  free_vector(&result);

clean_up:

  err_code = free_rec_block(root_block);
  if (err_code)
    printf("There was an error with free_rec_block: %d\n", err_code);
  err_code = finish_queries_state(&qs);
  if (err_code)
    printf("There was an error with finish_queries_state: %d\n", err_code);

  if (!err_code) {
    printf("No errors while freeing data\n");
  }

  return 0;
}

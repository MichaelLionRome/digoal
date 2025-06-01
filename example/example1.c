#include <block/block.h>
#include <block/queries_state.h>

#include <stdio.h>

int main(void) {
  uint32_t treedepth = 7;
  ulong side = 1u << treedepth;
  struct block *root_block = create_block(treedepth);

  struct queries_state qs;
  init_queries_state(&qs, treedepth);

  for (ulong col = 0; col < side; col++) {
    for (ulong row = 0; row < side; row++) {
      printf("inserting col=%lu, row=%lu\n", col, row);
      insert_point(root_block, col, row, &qs);
    }
  }

  free_rec_block(root_block);
  finish_queries_state(&qs);

  return 0;
}
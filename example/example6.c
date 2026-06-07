#include <block.h>
#include <queries_state.h>

#include <stdio.h>

int main(void) {
  uint32_t tree_depth = 3;
  struct block *root_block = create_block();
  struct queries_state qs;
  init_queries_state(&qs, tree_depth, MAX_NODES_IN_BLOCK, root_block);

  insert_point(root_block, 0, 0, &qs);

  int found_point;
  has_point(root_block, 0, 0, &qs, &found_point);

  finish_queries_state(&qs);
  free_block(root_block);
}

#include <block.h>
#include <queries_state.h>

#include <stdio.h>

int main(void) {

  uint32_t tree_depth = 32;
  struct block *root_block = create_block(tree_depth);
  struct queries_state qs;
  init_queries_state(&qs, tree_depth);

  FILE *fp = fopen("../test/points_output.txt", "r");

  unsigned long col, row;

  while (fscanf(fp, "%lu %lu", &col, &row) == 2) {
    insert_point(root_block, col, row, &qs);
    int has_it;
    has_point(root_block, col, row, &qs, &has_it);
    printf("Has point?: %s\n", has_it ? "YES" : "NO");
  }

  fclose(fp);

  return 0;
}
#include <block/block.h>
#include <block/queries_state.h>

#include <stdio.h>

int main(void) {
  for(int i = 0; i < 10; i++){
  uint32_t treedepth = 5;
  ulong side = 1u << treedepth;
  struct block *root_block = create_block(treedepth);

  struct queries_state qs;
  init_queries_state(&qs, treedepth);

  for (ulong col = 0; col < side; col++) {
    for (ulong row = 0; row < side; row++) {
      // printf("inserting col=%lu, row=%lu\n", col, row);
      insert_point(root_block, col, row, &qs);
    }
  }

  printf("Done inserting i=%d\n", i);

  //getchar();

  int err_code = free_rec_block(root_block);
  if(err_code)
    printf("There was an error with free_rec_block: %d\n", err_code);
  err_code = finish_queries_state(&qs);
  if(err_code)
    printf("There was an error with finish_queries_state: %d\n", err_code);

  if(!err_code){
    printf("No errors while freeing data\n");
  }

  //getchar();
  }

  return 0;
}

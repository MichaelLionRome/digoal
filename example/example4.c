#define _DEFAULT_SOURCE

#include <block.h>
#include <queries_state.h>

#include <stdio.h>

#include "definitions.h"
#include <sys/time.h>

int main(void) {
  int err_code;
  uint32_t treedepth = 11;
  uint32_t side = 1 << treedepth;
  struct block *root_block = create_block(treedepth);

  struct queries_state qs;
  init_queries_state(&qs, treedepth);

  struct timeval tval_before, tval_after, tval_result;

  gettimeofday(&tval_before, NULL);

  for (uint32_t col = 0; col < side; col++) {
    for (uint32_t row = 0; row < side; row++) {
      insert_point(root_block, col, row, &qs);
    }
  }

  gettimeofday(&tval_after, NULL);

  timersub(&tval_after, &tval_before, &tval_result);
  printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec,
         (long int)tval_result.tv_usec);

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

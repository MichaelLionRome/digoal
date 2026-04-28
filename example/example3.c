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
#include <block.h>
#include <queries_state.h>

#include <stdio.h>

#include "definitions.h"

int main(void) {
  int err_code;
  uint32_t treedepth = 25;
  struct block *root_block = create_block();

  struct queries_state qs;
  init_queries_state(&qs, treedepth, MAX_NODES_IN_BLOCK, root_block);

  struct vector_pair2dl_t input_pairs;
  vector_pair2dl_t__init_vector_with_capacity(&input_pairs, 1000);
  for (int i = 0; i < 1000; i++) {
    struct pair2dl ins = {i, i};
    vector_pair2dl_t__insert_element(&input_pairs, ins);
  }

  // struct pair2dl array_pairs[] = {{0, 0}, {3, 3}};
  int qty = input_pairs.nof_items;
  for (int i = 0; i < qty; i++) {
    struct pair2dl current = input_pairs.data[i];

    printf("inserting %lu, %lu\n", current.col, current.row);
    insert_point(root_block, current.col, current.row, &qs);
    int does_have_point;
    has_point(root_block, current.col, current.row, &qs, &does_have_point);
    if (!does_have_point) {
      printf("doesn't have point (%lu, %lu)\n", current.col, current.row);
      goto clean_up;
    }
  }

  struct vector_pair2dl_t result;
  vector_pair2dl_t__init_vector_with_capacity(&result, qty);

  err_code = naive_scan_points(root_block, &qs, &result);
  if (err_code) {
    exit(err_code);
  }

  printf("scanned points:\n");
  for (int i = 0; i < result.nof_items; i++) {
    struct pair2dl current = result.data[i];
    printf("element %d = (%lu, %lu)\n", i, current.col, current.row);
  }

  vector_pair2dl_t__free_vector(&result);

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

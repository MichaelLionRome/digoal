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

int main(void) {
  for (int i = 0; i < 10; i++) {
    uint32_t treedepth = 5;
    unsigned long side = 1u << treedepth;
    struct block *root_block = create_block();

    struct queries_state qs;
    init_queries_state(&qs, treedepth, MAX_NODES_IN_BLOCK, root_block);

    int point_exists;
    for (unsigned long col = 0; col < side; col++) {
      for (unsigned long row = 0; row < side; row++) {
        // printf("inserting col=%lu, row=%lu\n", col, row);
        insert_point(root_block, col, row, &qs, &point_exists);
      }
    }

    printf("Done inserting i=%d\n", i);

    // getchar();

    int err_code = free_rec_block(root_block);
    if (err_code)
      printf("There was an error with free_rec_block: %d\n", err_code);
    err_code = finish_queries_state(&qs);
    if (err_code)
      printf("There was an error with finish_queries_state: %d\n", err_code);

    if (!err_code) {
      printf("No errors while freeing data\n");
    }

    // getchar();
  }

  return 0;
}

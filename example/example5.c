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

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
#include <stdlib.h>

#include "bitvector.h"
#include "block.h"
#include "definitions.h"
#include "memalloc.h"

struct block *k2tree_alloc_block(void) {
  return (struct block *)malloc(sizeof(struct block));
}

uint32_t *k2tree_alloc_u32array(int size) {
  return (uint32_t *)calloc(size, sizeof(uint32_t));
}

int k2tree_free_block(struct block *b) {
  free(b);
  return SUCCESS_ECODE_K2T;
}

int k2tree_free_u32array(uint32_t *data, int size) {
  __UNUSED(size);
  free(data);
  return SUCCESS_ECODE_K2T;
}

void *k2tree_alloc_preorders(int capacity) {
  return malloc(sizeof(NODES_BV_T) * capacity);
}
struct block *k2tree_alloc_blocks_array(int capacity) {
  return (struct block *)malloc(sizeof(struct block) * capacity);
}
void k2tree_free_preorders(void *preorders) { free(preorders); }
void k2tree_free_blocks_array(struct block *blocks_array) {
  free(blocks_array);
}

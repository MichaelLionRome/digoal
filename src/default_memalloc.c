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

#include <bitvector.h>
#include <vector.h>

#include "memalloc.h"

#include "block.h"
#include "block_frontier.h"
#include "block_topology.h"

#include "definitions.h"

struct block *k2tree_alloc_block(void) {
  return (struct block *)malloc(sizeof(struct block));
}

struct block_topology *k2tree_alloc_block_topology(void) {
  return (struct block_topology *)malloc(sizeof(struct block_topology));
}

struct block_frontier *k2tree_alloc_block_frontier(void) {
  return (struct block_frontier *)malloc(sizeof(struct block_frontier));
}

struct bitvector *k2tree_alloc_bitvector(void) {
  return (struct bitvector *)malloc(sizeof(struct bitvector));
}

struct vector *k2tree_alloc_vector(void) {
  return (struct vector *)malloc(sizeof(struct vector));
}

struct u32array_alloc k2tree_alloc_u32array(int size) {
  struct u32array_alloc out;
  out.data = (uint32_t *)malloc(size * sizeof(uint32_t));
  out.size = size;
  return out;
}

int k2tree_free_block(struct block *b) {
  free(b);
  return SUCCESS_ECODE;
}

int k2tree_free_block_topology(struct block_topology *bt) {
  free(bt);
  return SUCCESS_ECODE;
}

int k2tree_free_block_frontier(struct block_frontier *bf) {
  free(bf);
  return SUCCESS_ECODE;
}

int k2tree_free_bitvector(struct bitvector *bv) {
  free(bv);
  return SUCCESS_ECODE;
}
int k2tree_free_vector(struct vector *v) {
  free(v);
  return SUCCESS_ECODE;
}

int k2tree_free_u32array(struct u32array_alloc alloc) {
  free(alloc.data);
  return SUCCESS_ECODE;
}

#include <stdlib.h>

#include <bitvector.h>
#include <vector.h>

#include "memalloc.h"

#include "block_frontier.h"
#include "block_topology.h"
#include "block.h"

#include "definitions.h"

struct block * __attribute__ ((noinline)) k2tree_alloc_block(void) {
  return (struct block *)calloc(1, sizeof(struct block));
}

struct block_topology * __attribute__ ((noinline)) k2tree_alloc_block_topology(void) {
  return (struct block_topology *)calloc(1, sizeof(struct block_topology));
}

struct block_frontier * __attribute__ ((noinline)) k2tree_alloc_block_frontier(void) {
  return (struct block_frontier *)calloc(1, sizeof(struct block_frontier));
}

struct bitvector * __attribute__ ((noinline)) k2tree_alloc_bitvector(void) {
  return (struct bitvector *)calloc(1, sizeof(struct bitvector));
}

struct vector * __attribute__ ((noinline)) k2tree_alloc_vector(void) {
  return (struct vector *)calloc(1, sizeof(struct vector));
}

struct u32array_alloc __attribute__ ((noinline)) k2tree_alloc_u32array(int size) {
  struct u32array_alloc out;
  out.data = (uint32_t *)calloc(size, sizeof(uint32_t));
  out.size = size;
  return out;
}

int __attribute__ ((noinline)) k2tree_free_block(struct block *b) {
  free(b);
  return SUCCESS_ECODE;
}

int __attribute__ ((noinline)) k2tree_free_block_topology(struct block_topology *bt) {
  free(bt);
  return SUCCESS_ECODE;
}

int __attribute__ ((noinline)) k2tree_free_block_frontier(struct block_frontier *bf) {
  free(bf);
  return SUCCESS_ECODE;
}

int __attribute__ ((noinline)) k2tree_free_bitvector(struct bitvector *bv) {
  free(bv);
  return SUCCESS_ECODE;
}
int __attribute__ ((noinline)) k2tree_free_vector(struct vector *v) {
  free(v);
  return SUCCESS_ECODE;
}

int __attribute__ ((noinline)) k2tree_free_u32array(struct u32array_alloc alloc) {
  free(alloc.data);
  return SUCCESS_ECODE;
}

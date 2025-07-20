#ifndef _MEMALLOC_H_
#define _MEMALLOC_H_

#include <stdint.h>

struct block;
struct block_topology;
struct block_frontier;

struct bitvector;
struct vector;

struct block *k2tree_alloc_block(void);
struct block_topology *k2tree_alloc_block_topology(void);
struct block_frontier *k2tree_alloc_block_frontier(void);

struct bitvector *k2tree_alloc_bitvector(void);
struct vector *k2tree_alloc_vector(void);

struct u32array_alloc {
  uint32_t *data;
  int size;
};

struct u32array_alloc k2tree_alloc_u32array(int size);

int k2tree_free_block(struct block *);
int k2tree_free_block_topology(struct block_topology *);
int k2tree_free_block_frontier(struct block_frontier *);

int k2tree_free_bitvector(struct bitvector *);
int k2tree_free_vector(struct vector *);

int k2tree_free_u32array(struct u32array_alloc);

#endif /* __MEMALLOC_H_ */

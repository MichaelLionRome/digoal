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
#ifndef _MEMALLOC_H_
#define _MEMALLOC_H_

#include <stdint.h>

struct block;
struct block_topology;
struct block_frontier;

struct bitvector;

struct block *k2tree_alloc_block(void);
struct block_topology *k2tree_alloc_block_topology(void);
struct block_frontier *k2tree_alloc_block_frontier(void);

struct bitvector *k2tree_alloc_bitvector(void);

struct u32array_alloc {
  uint32_t *data;
  int size;
};

struct u32array_alloc k2tree_alloc_u32array(int size);

int k2tree_free_block(struct block *);
int k2tree_free_block_topology(struct block_topology *);
int k2tree_free_block_frontier(struct block_frontier *);

int k2tree_free_bitvector(struct bitvector *);

int k2tree_free_u32array(struct u32array_alloc);

#endif /* __MEMALLOC_H_ */

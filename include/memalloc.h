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

uint32_t *k2tree_alloc_u32array(int size);

int k2tree_free_block(struct block *);
int k2tree_free_u32array(uint32_t *data, int size);

NODES_BV_T *k2tree_alloc_preorders(int capacity);
struct block *k2tree_alloc_blocks_array(int capacity);

void k2tree_free_preorders(NODES_BV_T *preorders);
void k2tree_free_blocks_array(struct block *blocks_array);
#endif /* __MEMALLOC_H_ */

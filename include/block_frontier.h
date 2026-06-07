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
#ifndef _BLOCK_FRONTIER_H
#define _BLOCK_FRONTIER_H

#include "vectors.h"

struct block;
/*
struct block {
  struct vector_uint16_t frontier;
  struct vector_block_ptr_t blocks;
};
*/

void init_block_frontier(struct block *input_block);
void init_block_frontier_with_capacity(struct block *input_block,
                                       uint32_t capacity);
void free_block_frontier(struct block *input_block);
int frontier_check(struct block *input_block, uint32_t node_idx,
                   uint32_t *frontier_traversal_idx);

struct block *get_child_block(struct block *input_block,
                              uint32_t frontier_node_idx);

int extract_sub_block_frontier(struct block *input_block,
                               uint32_t preorder_from, uint32_t preorder_to,
                               struct block *to_fill_bf);

int add_frontier_node(struct block *input_block,
                      uint32_t new_frontier_node_preorder, struct block *b);

int fix_frontier_indexes(struct block *input_block, uint32_t start, int delta);

int collapse_frontier_nodes(struct block *input_block, uint32_t from_preorder,
                            uint32_t to_preorder);

#endif /* _BLOCK_FRONTIER_H */

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
#ifndef _BLOCK_H_
#define _BLOCK_H_

#include "block_frontier.h"
#include "block_topology.h"
#include "vectors.h"
#include <stdint.h>

#include "queries_state.h"

typedef uint8_t TREE_DEPTH_T;
typedef uint16_t MAX_NODE_COUNT_T;
typedef uint32_t BLOCK_INDEX_T;

struct block {
  struct block_topology *bt;
  struct block_frontier *bf;
  TREE_DEPTH_T block_depth;
  TREE_DEPTH_T tree_depth;
  MAX_NODE_COUNT_T max_node_count;

  struct block *root;
  BLOCK_INDEX_T block_index;
};

typedef void (*point_reporter_fun_t)(ulong, ulong, void *);

int has_point(struct block *input_block, ulong col, ulong row,
              struct queries_state *qs, int *result);
int insert_point(struct block *input_block, ulong col, ulong row,
                 struct queries_state *qs);

int naive_scan_points(struct block *input_block, struct queries_state *qs,
                      struct vector_pair2dl_t *result);

int scan_points_interactively(struct block *input_block,
                              struct queries_state *qs,
                              point_reporter_fun_t point_reporter,
                              void *report_state);

int report_column(struct block *input_block, ulong col,
                  struct queries_state *qs, struct vector_pair2dl_t *result);
int report_row(struct block *input_block, ulong row, struct queries_state *qs,
               struct vector_pair2dl_t *result);

int report_column_interactively(struct block *input_block, ulong col,
                                struct queries_state *qs,
                                point_reporter_fun_t point_reporter,
                                void *report_state);
int report_row_interactively(struct block *input_block, ulong row,
                             struct queries_state *qs,
                             point_reporter_fun_t point_reporter,
                             void *report_state);

struct block *create_block(TREE_DEPTH_T tree_depth);
int free_rec_block(struct block *input_block);
int free_block(struct block *input_block);

#endif /* _BLOCK_H_ */

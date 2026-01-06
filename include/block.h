#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <stdint.h>

#include <vector.h>

#include "block_frontier.h"
#include "block_topology.h"

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
                      struct vector *result);

int scan_points_interactively(struct block *input_block,
                              struct queries_state *qs,
                              point_reporter_fun_t point_reporter,
                              void *report_state);

int report_column(struct block *input_block, ulong col,
                  struct queries_state *qs, struct vector *result);
int report_row(struct block *input_block, ulong row, struct queries_state *qs,
               struct vector *result);

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

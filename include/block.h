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
#include <stdint.h>

#include "bitvector.h"
#include "block_frontier.h"
#include "block_topology.h"
#include "definitions.h"
#include "queries_state.h"
#include "vectors.h"

/* definitions to simplify reporting */
#define REPORT_COLUMN 0
#define REPORT_ROW 1

#define REPORT_FIRST_HALF(which_report, child_pos)                             \
  (((which_report) == REPORT_COLUMN && (child_pos) < 2) ||                     \
   ((which_report) == REPORT_ROW && (child_pos) % 2 == 0))

#define REPORT_SECOND_HALF(which_report, child_pos)                            \
  (((which_report) == REPORT_COLUMN && (child_pos) >= 2) ||                    \
   ((which_report) == REPORT_ROW && (child_pos) % 2 == 1))

#define REPORT_CONTINUE_CONDITION(current_col, half_length, which_report,      \
                                  child_pos)                                   \
  (((current_col) < (half_length) &&                                           \
    REPORT_FIRST_HALF(which_report, child_pos)) ||                             \
   ((current_col) >= (half_length) &&                                          \
    REPORT_SECOND_HALF(which_report, child_pos)))

struct block {
  NODES_BV_T *preorders;
  struct block **children_blocks;

  BVCTYPE *container;

  NODES_BV_T children;
  CONTAINER_SZ_T container_size;
  NODES_BV_T nodes_count;
};

struct k2tree_measurement {
  unsigned long total_bytes;
  unsigned long total_blocks;
  unsigned long bytes_topology;
};

struct sip_join_input {
  struct block **blocks;
  struct queries_state **qss;
  struct sip_ipoint *join_coords;
  int join_size;

  struct sip_ipoint *_join_coords;
};

typedef void (*point_reporter_fun_t)(ulong, ulong, void *);
typedef void (*coord_reporter_fun_t)(ulong, void *);

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

int sip_join(struct sip_join_input input, coord_reporter_fun_t coord_reporter,
             void *report_state);

struct block *create_block(void);
int free_rec_block(struct block *input_block);
int free_block(struct block *input_block);

struct k2tree_measurement measure_tree_size(struct block *input_block);

#endif /* _BLOCK_H_ */

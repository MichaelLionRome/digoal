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
#ifndef _K2NODE_H_
#define _K2NODE_H_

#include "block.h"
#include "vectors.h"

struct k2qstate {
  struct queries_state qs;
  TREE_DEPTH_T k2tree_depth;
  TREE_DEPTH_T cut_depth;
  struct morton_code mc;
};

struct k2node {
  union {
    struct k2node *children[4];
    struct block *block_child;
  } k2subtree;
};

int k2node_has_point(struct k2node *k2node, unsigned long col,
                     unsigned long row, struct k2qstate *st, int *result);
int k2node_insert_point(struct k2node *input_node, unsigned long col,
                        unsigned long row, struct k2qstate *st,
                        int *already_exists);
int k2node_delete_point(struct k2node *input_node, unsigned long col,
                        unsigned long row, struct k2qstate *st,
                        int *already_not_exists);

int k2node_naive_scan_points(struct k2node *input_node, struct k2qstate *st,
                             struct vector_pair2dl_t *result);

int k2node_scan_points_interactively(struct k2node *input_node,
                                     struct k2qstate *st,
                                     point_reporter_fun_t point_reporter,
                                     void *report_state);

int k2node_report_column(struct k2node *input_node, unsigned long col,
                         struct k2qstate *st, struct vector_pair2dl_t *result);
int k2node_report_row(struct k2node *input_node, unsigned long row,
                      struct k2qstate *st, struct vector_pair2dl_t *result);

int k2node_report_column_interactively(struct k2node *input_node,
                                       unsigned long col, struct k2qstate *st,
                                       point_reporter_fun_t point_reporter,
                                       void *report_state);
int k2node_report_row_interactively(struct k2node *input_node,
                                    unsigned long row, struct k2qstate *st,
                                    point_reporter_fun_t point_reporter,
                                    void *report_state);

struct k2node *create_k2node(void);
int free_rec_k2node(struct k2node *input_node, unsigned long current_depth,
                    unsigned long cut_depth);

int init_k2qstate(struct k2qstate *st, TREE_DEPTH_T treedepth,
                  MAX_NODE_COUNT_T max_nodes_count, TREE_DEPTH_T cut_depth);
int clean_k2qstate(struct k2qstate *st);
struct k2tree_measurement k2node_measure_tree_size(struct k2node *input_node,
                                                   unsigned long cut_depth);

int debug_validate_k2node(struct k2node *input_node, struct k2qstate *st,
                          TREE_DEPTH_T current_depth);
int debug_validate_k2node_rec(struct k2node *input_node, struct k2qstate *st,
                              TREE_DEPTH_T current_depth);
typedef struct {
  struct k2node *input_node;
  uint32_t last_iteration;
  unsigned long current_depth;
} k2node_lazy_naive_state;

define_stack_of_type(k2node_lazy_naive_state)

    struct k2node_lazy_handler_naive_scan_t {
  struct k2qstate *st;
  struct k2node_lazy_naive_state_stack states_stack;
  pair2dl_t next_result;
  int has_next;
  struct lazy_handler_naive_scan_t sub_handler;
  int at_leaf;
  unsigned long base_col;
  unsigned long base_row;

  struct k2node *tree_root;
};

typedef struct {
  struct k2node *input_node;
  uint64_t current_coord;
  uint32_t last_iteration;
  unsigned long current_depth;
} k2node_lazy_report_band_state_t;

define_stack_of_type(k2node_lazy_report_band_state_t)

    struct k2node_lazy_handler_report_band_t {
  struct k2qstate *st;
  struct k2node_lazy_report_band_state_t_stack stack;
  struct lazy_handler_report_band_t sub_handler;
  int which_report;
  int at_leaf;
  uint64_t next_result;
  int has_next;
  unsigned long base_col;
  unsigned long base_row;
  struct k2node *tree_root;
  uint64_t coord_report;
};

int k2node_naive_scan_points_lazy_init(
    struct k2node *input_node, struct k2qstate *st,
    struct k2node_lazy_handler_naive_scan_t *lazy_handler);

int k2node_naive_scan_points_lazy_clean(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler);

int k2node_naive_scan_points_lazy_next(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler, pair2dl_t *result);

int k2node_naive_scan_points_lazy_has_next(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler, int *result);

int k2node_naive_scan_points_lazy_reset(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler);

int k2node_report_row_lazy_init(
    struct k2node_lazy_handler_report_band_t *lazy_handler,
    struct k2node *input_node, struct k2qstate *st, uint64_t coord);
int k2node_report_column_lazy_init(
    struct k2node_lazy_handler_report_band_t *lazy_handler,
    struct k2node *input_node, struct k2qstate *st, uint64_t coord);

int k2node_report_band_lazy_clean(
    struct k2node_lazy_handler_report_band_t *lazy_handler);
int k2node_report_band_next(
    struct k2node_lazy_handler_report_band_t *lazy_handler, uint64_t *result);

int k2node_report_band_has_next(
    struct k2node_lazy_handler_report_band_t *lazy_handler, int *result);

int k2node_report_band_reset(
    struct k2node_lazy_handler_report_band_t *lazy_handler);

int print_debug_k2node(struct k2node *node, struct k2qstate *st);

#endif

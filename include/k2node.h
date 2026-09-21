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

struct k2node_sip_input{
  struct k2node ** nodes;
  struct sip_ipoint *join_coords;
  int join_size;
};

int k2node_has_point(struct k2node *k2node, ulong col, ulong row,
                     struct k2qstate *st, int *result);
int k2node_insert_point(struct k2node *input_node, ulong col, ulong row,
                        struct k2qstate *st);

int k2node_naive_scan_points(struct k2node *input_node, struct k2qstate *st,
                             struct vector_pair2dl_t *result);

int k2node_scan_points_interactively(struct k2node *input_node,
                                     struct k2qstate *st,
                                     point_reporter_fun_t point_reporter,
                                     void *report_state);

int k2node_report_column(struct k2node *input_node, ulong col,
                         struct k2qstate *st, struct vector_pair2dl_t *result);
int k2node_report_row(struct k2node *input_node, ulong row, struct k2qstate *st,
                      struct vector_pair2dl_t *result);

int k2node_report_column_interactively(struct k2node *input_node, ulong col,
                                       struct k2qstate *st,
                                       point_reporter_fun_t point_reporter,
                                       void *report_state);
int k2node_report_row_interactively(struct k2node *input_node, ulong row,
                                    struct k2qstate *st,
                                    point_reporter_fun_t point_reporter,
                                    void *report_state);



struct k2node *create_k2node(void);
int free_rec_k2node(struct k2node *input_node, ulong current_depth,
                    ulong cut_depth);

int init_k2qstate(struct k2qstate *st, TREE_DEPTH_T treedepth,
                  MAX_NODE_COUNT_T max_nodes_count, TREE_DEPTH_T cut_depth);
int clean_k2qstate(struct k2qstate *st);
struct k2tree_measurement k2node_measure_tree_size(struct k2node *input_node,
                                                   ulong cut_depth);

#endif


#include <stdint.h>
#include <stdlib.h>

#include "definitions.h"
#include "k2node.h"

struct k2_find_subtree_result {
  int exists;
  struct block *subtree_root;
  ulong col;
  ulong row;
  ulong depth_reached;
  struct k2node *last_node_visited;
};

struct k2_next_level_data {
  ulong quadrant;
  ulong col;
  ulong row;
};

/* private prototypes */
struct k2_next_level_data find_next_level_data(ulong col, ulong row,
                                               ulong remaining_depth);
struct k2_find_subtree_result k2_find_subtree(struct k2node *node,
                                              struct k2qstate *st, ulong col,
                                              ulong row, ulong current_depth);
int k2node_naive_scan_points_rec(struct k2node *node, struct k2qstate *st,
                                 ulong current_depth,
                                 struct vector_pair2dl_t *result);
int k2node_scan_points_interactively_rec(struct k2node *node,
                                         struct k2qstate *st,
                                         ulong current_depth,
                                         point_reporter_fun_t point_reporter,
                                         void *report_state);
int k2node_report_rec(struct k2node *node, ulong coord, int which_report,
                      ulong current_depth, struct k2qstate *st,
                      struct vector_pair2dl_t *result);
int k2node_report_interactively_rec(struct k2node *node, ulong coord,
                                    int which_report, ulong current_depth,
                                    struct k2qstate *st,
                                    point_reporter_fun_t point_reporter,
                                    void *report_state);
struct k2_find_subtree_result fill_insertion_path(struct k2node *from_node,
                                                  ulong col, ulong row,
                                                  struct k2qstate *st,
                                                  ulong current_depth);
struct k2tree_measurement
k2node_measure_tree_size_rec(struct k2node *input_node, ulong current_depth,
                             ulong cut_depth);
/* private implementations */
struct k2_next_level_data find_next_level_data(ulong col, ulong row,
                                               ulong remaining_depth) {
  ulong next_remaining_depth = remaining_depth - 1;
  ulong half_level = (next_remaining_depth << 1);
  uint32_t quadrant;

  if (col >= half_level && row >= half_level) {
    quadrant = 3;
  } else if (col >= half_level && row < half_level) {
    quadrant = 2;
  } else if (col < half_level && row >= half_level) {
    quadrant = 1;
  } else {
    quadrant = 0;
  }

  struct k2_next_level_data result;

  result.col = col % half_level;
  result.row = row % half_level;
  result.quadrant = quadrant;
  return result;
}

struct k2_find_subtree_result k2_find_subtree(struct k2node *node,
                                              struct k2qstate *st, ulong col,
                                              ulong row, ulong current_depth) {
  ulong remaining_depth = st->k2tree_depth - current_depth;
  if (current_depth == st->cut_depth) {
    struct k2_find_subtree_result result;
    result.col = col;
    result.row = row;
    result.subtree_root = node->k2subtree.block_child;
    result.exists = node->k2subtree.block_child != NULL;
    result.last_node_visited = node;
    result.depth_reached = current_depth;
    return result;
  }

  struct k2_next_level_data next_level_data =
      find_next_level_data(col, row, remaining_depth);

  struct k2node *next_node = node->k2subtree.children[next_level_data.quadrant];
  if (next_node == NULL) {
    struct k2_find_subtree_result result;
    result.exists = FALSE;
    result.last_node_visited = node;
    result.depth_reached = current_depth;
    result.col = col;
    result.row = row;
    result.subtree_root = NULL;
    return result;
  }

  return k2_find_subtree(next_node, st, next_level_data.col,
                         next_level_data.row, current_depth + 1);
}

int k2node_naive_scan_points_rec(struct k2node *node, struct k2qstate *st,
                                 ulong current_depth,
                                 struct vector_pair2dl_t *result) {
  if (current_depth == st->cut_depth) {
    return naive_scan_points(node->k2subtree.block_child, st->qs, result);
  }

  for (int child_index = 0; child_index < 4; child_index++) {
    if (node->k2subtree.children[child_index] != NULL) {
      CHECK_ERR(
          k2node_naive_scan_points_rec(node->k2subtree.children[child_index],
                                       st, current_depth + 1, result));
    }
  }

  return SUCCESS_ECODE_K2T;
}

int k2node_scan_points_interactively_rec(struct k2node *node,
                                         struct k2qstate *st,
                                         ulong current_depth,
                                         point_reporter_fun_t point_reporter,
                                         void *report_state) {
  if (current_depth == st->cut_depth) {
    return scan_points_interactively(node->k2subtree.block_child, st->qs,
                                     point_reporter, report_state);
  }

  for (int child_index = 0; child_index < 4; child_index++) {
    if (node->k2subtree.children[child_index] != NULL) {
      CHECK_ERR(k2node_scan_points_interactively_rec(
          node->k2subtree.children[child_index], st, current_depth + 1,
          point_reporter, report_state));
    }
  }

  return SUCCESS_ECODE_K2T;
}

int k2node_report_rec(struct k2node *node, ulong coord, int which_report,
                      ulong current_depth, struct k2qstate *st,
                      struct vector_pair2dl_t *result) {

  ulong remaining_depth = st->k2tree_depth - current_depth;
  if (current_depth == st->cut_depth) {
    if (which_report == REPORT_COLUMN) {
      return report_column(node->k2subtree.block_child, coord, st->qs, result);
    } else {
      return report_row(node->k2subtree.block_child, coord, st->qs, result);
    }
  }

  ulong next_remaining_depth = remaining_depth - 1;
  ulong half_level = (next_remaining_depth << 1);

  for (int child_pos = 0; child_pos < 4; child_pos++) {
    if (!REPORT_CONTINUE_CONDITION(coord, half_level, which_report,
                                   child_pos) ||
        !node->k2subtree.children[child_pos])
      continue;

    CHECK_ERR(k2node_report_rec(node->k2subtree.children[child_pos],
                                coord % half_level, which_report,
                                current_depth + 1, st, result));
  }

  return SUCCESS_ECODE_K2T;
}

int k2node_report_interactively_rec(struct k2node *node, ulong coord,
                                    int which_report, ulong current_depth,
                                    struct k2qstate *st,
                                    point_reporter_fun_t point_reporter,
                                    void *report_state) {

  ulong remaining_depth = st->k2tree_depth - current_depth;
  if (current_depth == st->cut_depth) {
    if (which_report == REPORT_COLUMN) {
      return report_column_interactively(node->k2subtree.block_child, coord,
                                         st->qs, point_reporter, report_state);
    } else {
      return report_row_interactively(node->k2subtree.block_child, coord,
                                      st->qs, point_reporter, report_state);
    }
  }

  ulong next_remaining_depth = remaining_depth - 1;
  ulong half_level = (next_remaining_depth << 1);

  for (int child_pos = 0; child_pos < 4; child_pos++) {
    if (!REPORT_CONTINUE_CONDITION(coord, half_level, which_report,
                                   child_pos) ||
        !node->k2subtree.children[child_pos])
      continue;

    CHECK_ERR(k2node_report_interactively_rec(
        node->k2subtree.children[child_pos], coord % half_level, which_report,
        current_depth + 1, st, point_reporter, report_state));
  }

  return SUCCESS_ECODE_K2T;
}

struct k2_find_subtree_result fill_insertion_path(struct k2node *from_node,
                                                  ulong col, ulong row,
                                                  struct k2qstate *st,
                                                  ulong current_depth) {
  ulong remaining_depth = st->k2tree_depth - current_depth;
  if (current_depth == st->cut_depth) {
    struct k2_find_subtree_result result;
    result.col = col;
    result.row = row;
    result.depth_reached = current_depth;
    result.exists = TRUE;
    result.last_node_visited = from_node;
    result.subtree_root = create_block();
    from_node->k2subtree.block_child = result.subtree_root;
    return result;
  }

  struct k2_next_level_data next_level_data =
      find_next_level_data(col, row, remaining_depth);

  if (from_node->k2subtree.children[next_level_data.quadrant]) {
    fprintf(stderr, "debug: fill_insertion_path bad case %p\n",
            (void *)from_node->k2subtree.children[next_level_data.quadrant]);
    exit(1);
  }

  from_node->k2subtree.children[next_level_data.quadrant] = create_k2node();
  return fill_insertion_path(
      from_node->k2subtree.children[next_level_data.quadrant],
      next_level_data.col, next_level_data.row, st, current_depth + 1);
}

struct k2tree_measurement
k2node_measure_tree_size_rec(struct k2node *input_node, ulong current_depth,
                             ulong cut_depth) {
  if (current_depth == cut_depth) {
    if (!input_node->k2subtree.block_child) {
      struct k2tree_measurement measurement;
      measurement.bytes_topology = 0;
      measurement.total_blocks = 0;
      measurement.total_bytes = sizeof(struct k2node);
      return measurement;
    }
    struct k2tree_measurement measurement =
        measure_tree_size(input_node->k2subtree.block_child);
    measurement.total_bytes += sizeof(struct k2node);
    return measurement;
  }

  struct k2tree_measurement measurement;
  measurement.total_bytes = 0;
  measurement.total_blocks = 0;
  measurement.bytes_topology = 0;

  for (int child_pos = 0; child_pos < 4; child_pos++) {
    struct k2node *child_node = input_node->k2subtree.children[child_pos];
    if (child_node) {
      struct k2tree_measurement child_measurement =
          k2node_measure_tree_size_rec(child_node, current_depth + 1,
                                       cut_depth);
      measurement.total_bytes += child_measurement.total_bytes;
      measurement.total_blocks += child_measurement.total_blocks;
      measurement.bytes_topology += child_measurement.bytes_topology;
    }
  }

  measurement.total_bytes += sizeof(struct k2node);

  return measurement;
}
/* public implementations */

int k2node_has_point(struct k2node *root_node, ulong col, ulong row,
                     struct k2qstate *st, int *result) {
  struct k2_find_subtree_result tr_result =
      k2_find_subtree(root_node, st, col, row, 0);
  if (!tr_result.exists) {
    *result = FALSE;
    return SUCCESS_ECODE_K2T;
  }
  return has_point(tr_result.subtree_root, tr_result.col, tr_result.row, st->qs,
                   result);
}

int k2node_insert_point(struct k2node *root_node, ulong col, ulong row,
                        struct k2qstate *st) {
  struct k2_find_subtree_result tr_result =
      k2_find_subtree(root_node, st, col, row, 0);
  if (!tr_result.exists) {
    tr_result = fill_insertion_path(tr_result.last_node_visited, tr_result.col,
                                    tr_result.row, st, tr_result.depth_reached);
  }
  st->qs->root = tr_result.subtree_root;
  return insert_point(tr_result.subtree_root, tr_result.col, tr_result.row,
                      st->qs);
}

int k2node_naive_scan_points(struct k2node *input_node, struct k2qstate *st,
                             struct vector_pair2dl_t *result) {
  return k2node_naive_scan_points_rec(input_node, st, 0, result);
}

int k2node_scan_points_interactively(struct k2node *input_node,
                                     struct k2qstate *st,
                                     point_reporter_fun_t point_reporter,
                                     void *report_state) {
  return k2node_scan_points_interactively_rec(input_node, st, 0, point_reporter,
                                              report_state);
}

int k2node_report_column(struct k2node *input_node, ulong col,
                         struct k2qstate *st, struct vector_pair2dl_t *result) {
  return k2node_report_rec(input_node, col, REPORT_COLUMN, 0, st, result);
}

int k2node_report_row(struct k2node *input_node, ulong row, struct k2qstate *st,
                      struct vector_pair2dl_t *result) {
  return k2node_report_rec(input_node, row, REPORT_ROW, 0, st, result);
}

int k2node_report_column_interactively(struct k2node *input_node, ulong col,
                                       struct k2qstate *st,
                                       point_reporter_fun_t point_reporter,
                                       void *report_state) {
  return k2node_report_interactively_rec(input_node, col, REPORT_COLUMN, 0, st,
                                         point_reporter, report_state);
}

struct k2node *create_k2node(void) {
  return (struct k2node *)calloc(1, sizeof(struct k2node));
}

int free_rec_k2node(struct k2node *input_node, ulong current_depth,
                    ulong cut_depth) {
  if (current_depth == cut_depth) {
    free_rec_block(input_node->k2subtree.block_child);
  } else {
    for (int child_pos = 0; child_pos < 4; child_pos++) {
      struct k2node *child_node = input_node->k2subtree.children[child_pos];
      if (child_node)
        free_rec_k2node(child_node, current_depth + 1, cut_depth);
    }
  }

  free(input_node);
  return SUCCESS_ECODE_K2T;
}

struct k2tree_measurement k2node_measure_tree_size(struct k2node *input_node,
                                                   ulong cut_depth) {
  return k2node_measure_tree_size_rec(input_node, 0, cut_depth);
}


#include <stdint.h>
#include <stdlib.h>

#include "definitions.h"
#include "k2node.h"

struct k2_find_subtree_result {
  int exists;
  struct block *subtree_root;
  unsigned long col;
  unsigned long row;
  unsigned long depth_reached;
  struct k2node *last_node_visited;
};

struct interactive_report_data {
  point_reporter_fun_t point_reporter;
  void *report_state;
  unsigned long base_col;
  unsigned long base_row;
};

/* private prototypes */
struct k2_find_subtree_result
k2_find_subtree(struct k2node *node, struct k2qstate *st, unsigned long col,
                unsigned long row, unsigned long current_depth);
int k2node_naive_scan_points_rec(struct k2node *node, struct k2qstate *st,
                                 unsigned long current_depth,
                                 struct vector_pair2dl_t *result);
int k2node_scan_points_interactively_rec(struct k2node *node,
                                         struct k2qstate *st,
                                         unsigned long current_depth,
                                         point_reporter_fun_t point_reporter,
                                         void *report_state);
int k2node_report_rec(struct k2node *node, unsigned long coord,
                      int which_report, unsigned long current_depth,
                      struct k2qstate *st, struct vector_pair2dl_t *result);
int k2node_report_interactively_rec(struct k2node *node, unsigned long coord,
                                    int which_report,
                                    unsigned long current_depth,
                                    struct k2qstate *st,
                                    point_reporter_fun_t point_reporter,
                                    void *report_state);
struct k2_find_subtree_result fill_insertion_path(struct k2node *from_node,
                                                  unsigned long col,
                                                  unsigned long row,
                                                  struct k2qstate *st,
                                                  unsigned long current_depth);
struct k2tree_measurement
k2node_measure_tree_size_rec(struct k2node *input_node,
                             unsigned long current_depth,
                             unsigned long cut_depth);

void interactive_transform_points(unsigned long col, unsigned long row,
                                  void *data);

int k2node_sip_join_rec(struct k2node_sip_input input,
                        TREE_DEPTH_T current_depth, TREE_DEPTH_T treedepth,
                        TREE_DEPTH_T cut_depth, struct morton_code *mc,
                        struct k2node **current_nodes,
                        struct sip_ipoint *join_coords,
                        coord_reporter_fun_t coord_reporter,
                        void *report_state);

int k2node_has_more_than_one_child(struct k2node *input_node, int current_depth,
                                   int cut_depth);

int k2node_delete_point_rec(struct k2node *input_node, struct k2qstate *st,
                            unsigned long col, unsigned long row,
                            int current_depth, int *already_not_exists,
                            int *has_children);

void sip_select_child(long coord, coord_t coord_type, int *selected_children,
                      long half_length);
/* private implementations */

struct k2_find_subtree_result
k2_find_subtree(struct k2node *node, struct k2qstate *st, unsigned long col,
                unsigned long row, unsigned long current_depth) {
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

  uint32_t child_pos = get_code_at_morton_code(&st->mc, current_depth);

  struct k2node *next_node = node->k2subtree.children[child_pos];
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
  unsigned long remaining_depth = st->k2tree_depth - current_depth;
  unsigned long half_length = 1UL << (remaining_depth - 1UL);

  return k2_find_subtree(next_node, st, col % half_length, row % half_length,
                         current_depth + 1);
}

int k2node_naive_scan_points_rec(struct k2node *node, struct k2qstate *st,
                                 unsigned long current_depth,
                                 struct vector_pair2dl_t *result) {
  if (current_depth == st->cut_depth) {
    struct pair2dl high_level_coordinates;
    long starting_pos = result->nof_items;
    convert_morton_code_to_coordinates_select_treedepth(
        &st->mc, &high_level_coordinates, st->cut_depth);
    CHECK_ERR(naive_scan_points(node->k2subtree.block_child, &st->qs, result));
    unsigned long base_col = high_level_coordinates.col
                             << (st->k2tree_depth - st->cut_depth);
    unsigned long base_row = high_level_coordinates.row
                             << (st->k2tree_depth - st->cut_depth);
    for (; starting_pos < result->nof_items; starting_pos++) {
      result->data[starting_pos].col += base_col;
      result->data[starting_pos].row += base_row;
    }
    return SUCCESS_ECODE_K2T;
  }

  for (uint32_t child_index = 0; child_index < 4; child_index++) {
    if (node->k2subtree.children[child_index]) {
      add_element_morton_code(&st->mc, current_depth, child_index);
      CHECK_ERR(
          k2node_naive_scan_points_rec(node->k2subtree.children[child_index],
                                       st, current_depth + 1, result));
    }
  }

  return SUCCESS_ECODE_K2T;
}

void interactive_transform_points(unsigned long col, unsigned long row,
                                  void *data) {
  struct interactive_report_data *middle_state =
      (struct interactive_report_data *)data;
  middle_state->point_reporter(middle_state->base_col + col,
                               middle_state->base_row + row,
                               middle_state->report_state);
}

int k2node_scan_points_interactively_rec(struct k2node *node,
                                         struct k2qstate *st,
                                         unsigned long current_depth,
                                         point_reporter_fun_t point_reporter,
                                         void *report_state) {
  if (current_depth == st->cut_depth) {
    struct pair2dl high_level_coordinates;
    convert_morton_code_to_coordinates_select_treedepth(
        &st->mc, &high_level_coordinates, st->cut_depth);
    struct interactive_report_data middle_state;
    middle_state.point_reporter = point_reporter;
    middle_state.report_state = report_state;
    middle_state.base_col =
        high_level_coordinates.col
        << ((unsigned long)st->k2tree_depth - (unsigned long)st->cut_depth);
    middle_state.base_row =
        high_level_coordinates.row
        << ((unsigned long)st->k2tree_depth - (unsigned long)st->cut_depth);
    return scan_points_interactively(node->k2subtree.block_child, &st->qs,
                                     interactive_transform_points,
                                     &middle_state);
  }

  for (int child_index = 0; child_index < 4; child_index++) {
    if (node->k2subtree.children[child_index] != NULL) {
      add_element_morton_code(&st->mc, current_depth, child_index);
      CHECK_ERR(k2node_scan_points_interactively_rec(
          node->k2subtree.children[child_index], st, current_depth + 1,
          point_reporter, report_state));
    }
  }

  return SUCCESS_ECODE_K2T;
}

int k2node_report_rec(struct k2node *node, unsigned long coord,
                      int which_report, unsigned long current_depth,
                      struct k2qstate *st, struct vector_pair2dl_t *result) {

  unsigned long remaining_depth = st->k2tree_depth - current_depth;
  if (current_depth == st->cut_depth) {
    struct pair2dl high_level_coordinates;
    convert_morton_code_to_coordinates_select_treedepth(
        &st->mc, &high_level_coordinates, st->cut_depth);
    uint32_t starting_pos = result->nof_items;
    unsigned long base_col = high_level_coordinates.col
                             << (st->k2tree_depth - st->cut_depth);
    unsigned long base_row = high_level_coordinates.row
                             << (st->k2tree_depth - st->cut_depth);
    if (which_report == REPORT_COLUMN) {
      CHECK_ERR(
          report_column(node->k2subtree.block_child, coord, &st->qs, result));
    } else {
      CHECK_ERR(
          report_row(node->k2subtree.block_child, coord, &st->qs, result));
    }

    for (; starting_pos < result->nof_items; starting_pos++) {
      result->data[starting_pos].col += base_col;
      result->data[starting_pos].row += base_row;
    }
    return SUCCESS_ECODE_K2T;
  }

  unsigned long next_remaining_depth = remaining_depth - 1;
  unsigned long half_level = 1 << next_remaining_depth;

  for (int child_pos = 0; child_pos < 4; child_pos++) {
    if (!node->k2subtree.children[child_pos] ||
        !REPORT_CONTINUE_CONDITION(coord, half_level, which_report, child_pos))
      continue;

    add_element_morton_code(&st->mc, current_depth, child_pos);
    CHECK_ERR(k2node_report_rec(node->k2subtree.children[child_pos],
                                coord % half_level, which_report,
                                current_depth + 1, st, result));
  }

  return SUCCESS_ECODE_K2T;
}

int k2node_report_interactively_rec(struct k2node *node, unsigned long coord,
                                    int which_report,
                                    unsigned long current_depth,
                                    struct k2qstate *st,
                                    point_reporter_fun_t point_reporter,
                                    void *report_state) {

  unsigned long remaining_depth = st->k2tree_depth - current_depth;
  if (current_depth == st->cut_depth) {
    struct pair2dl high_level_coordinates;
    convert_morton_code_to_coordinates_select_treedepth(
        &st->mc, &high_level_coordinates, st->cut_depth);
    struct interactive_report_data middle_state;
    middle_state.point_reporter = point_reporter;
    middle_state.report_state = report_state;
    middle_state.base_col = high_level_coordinates.col
                            << (st->k2tree_depth - st->cut_depth);
    middle_state.base_row = high_level_coordinates.row
                            << (st->k2tree_depth - st->cut_depth);
    if (which_report == REPORT_COLUMN) {
      CHECK_ERR(report_column_interactively(
          node->k2subtree.block_child, coord, &st->qs,
          interactive_transform_points, &middle_state));
    } else {
      CHECK_ERR(report_row_interactively(node->k2subtree.block_child, coord,
                                         &st->qs, interactive_transform_points,
                                         &middle_state));
    }
    return SUCCESS_ECODE_K2T;
  }

  unsigned long next_remaining_depth = remaining_depth - 1;
  unsigned long half_level = 1 << next_remaining_depth;

  for (int child_pos = 0; child_pos < 4; child_pos++) {
    if (!REPORT_CONTINUE_CONDITION(coord, half_level, which_report,
                                   child_pos) ||
        !node->k2subtree.children[child_pos])
      continue;
    add_element_morton_code(&st->mc, current_depth, child_pos);
    CHECK_ERR(k2node_report_interactively_rec(
        node->k2subtree.children[child_pos], coord % half_level, which_report,
        current_depth + 1, st, point_reporter, report_state));
  }

  return SUCCESS_ECODE_K2T;
}

struct k2_find_subtree_result fill_insertion_path(struct k2node *from_node,
                                                  unsigned long col,
                                                  unsigned long row,
                                                  struct k2qstate *st,
                                                  unsigned long current_depth) {
  unsigned long remaining_depth = st->k2tree_depth - current_depth;
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

  uint32_t child_pos = get_code_at_morton_code(&st->mc, current_depth);

  if (from_node->k2subtree.children[child_pos]) {
    fprintf(stderr, "debug: fill_insertion_path bad case %p\n",
            (void *)from_node->k2subtree.children[child_pos]);
    exit(1);
  }

  from_node->k2subtree.children[child_pos] = create_k2node();

  unsigned long half_length = 1UL << (remaining_depth - 1UL);

  return fill_insertion_path(from_node->k2subtree.children[child_pos],
                             col % half_length, row % half_length, st,
                             current_depth + 1);
}

struct k2tree_measurement
k2node_measure_tree_size_rec(struct k2node *input_node,
                             unsigned long current_depth,
                             unsigned long cut_depth) {
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

int k2node_has_more_than_one_child(struct k2node *input_node, int current_depth,
                                   int cut_depth) {
  if (current_depth == cut_depth) {
    return input_node->k2subtree.block_child != NULL;
  }
  int count = 0;
  for (int i = 0; i < 4; i++) {
    if (input_node->k2subtree.children[i])
      count++;
    if (count > 1)
      return TRUE;
  }
  return FALSE;
}

int k2node_delete_point_rec(struct k2node *input_node, struct k2qstate *st,
                            unsigned long col, unsigned long row,
                            int current_depth, int *already_not_exists,
                            int *has_children) {

  if (current_depth == st->cut_depth) {
    struct block *block_tree = input_node->k2subtree.block_child;

    if (block_tree == NULL) {
      *already_not_exists = TRUE;
      return SUCCESS_ECODE_K2T;
    }

    CHECK_ERR(delete_point(block_tree, col, row, &st->qs, already_not_exists));

    if (block_tree->nodes_count == 0) {
      k2tree_free_block(block_tree);
      *has_children = FALSE;
      input_node->k2subtree.block_child = NULL;
    }
    return SUCCESS_ECODE_K2T;
  }

  unsigned long remaining_depth = st->k2tree_depth - current_depth;
  unsigned long half_length = 1 << (remaining_depth - 1);

  uint32_t child_pos = get_code_at_morton_code(&st->mc, current_depth);
  struct k2node *next_node = input_node->k2subtree.children[child_pos];
  if (next_node == NULL) {
    *already_not_exists = TRUE;
    return SUCCESS_ECODE_K2T;
  }

  int next_depth = current_depth + 1;
  CHECK_ERR(k2node_delete_point_rec(next_node, st, col % half_length,
                                    row % half_length, next_depth,
                                    already_not_exists, has_children));

  if (*already_not_exists || *has_children)
    return SUCCESS_ECODE_K2T;

  if (!k2node_has_more_than_one_child(next_node, next_depth, st->cut_depth)) {
    k2tree_free_k2node(next_node);
    input_node->k2subtree.children[child_pos] = NULL;
  } else {
    *has_children = TRUE;
  }

  return SUCCESS_ECODE_K2T;
}

void sip_select_child(long coord, coord_t coord_type, int *selected_children,
                      long half_length) {
  switch (coord_type) {
  case COLUMN_COORD:
    if (coord < half_length) {
      selected_children[0] = 0;
      selected_children[1] = 1;
    } else {
      selected_children[0] = 2;
      selected_children[1] = 3;
    }
    break;
  case ROW_COORD:
  default:
    if (coord < half_length) {
      selected_children[0] = 0;
      selected_children[1] = 2;
    } else {
      selected_children[0] = 1;
      selected_children[1] = 3;
    }
    break;
  }
}

/* public implementations */

int k2node_has_point(struct k2node *root_node, unsigned long col,
                     unsigned long row, struct k2qstate *st, int *result) {
  convert_coordinates_to_morton_code(col, row, st->k2tree_depth, &st->mc);
  struct k2_find_subtree_result tr_result =
      k2_find_subtree(root_node, st, col, row, 0);
  if (!tr_result.exists) {
    *result = FALSE;
    return SUCCESS_ECODE_K2T;
  }
  return has_point(tr_result.subtree_root, tr_result.col, tr_result.row,
                   &st->qs, result);
}

int k2node_insert_point(struct k2node *root_node, unsigned long col,
                        unsigned long row, struct k2qstate *st,
                        int *already_exists) {
  convert_coordinates_to_morton_code(col, row, st->k2tree_depth, &st->mc);
  struct k2_find_subtree_result tr_result =
      k2_find_subtree(root_node, st, col, row, 0);
  if (!tr_result.exists) {
    tr_result = fill_insertion_path(tr_result.last_node_visited, tr_result.col,
                                    tr_result.row, st, tr_result.depth_reached);
  }
  st->qs.root = tr_result.subtree_root;
  return insert_point(tr_result.subtree_root, tr_result.col, tr_result.row,
                      &st->qs, already_exists);
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

int k2node_report_column(struct k2node *input_node, unsigned long col,
                         struct k2qstate *st, struct vector_pair2dl_t *result) {
  return k2node_report_rec(input_node, col, REPORT_COLUMN, 0, st, result);
}

int k2node_report_row(struct k2node *input_node, unsigned long row,
                      struct k2qstate *st, struct vector_pair2dl_t *result) {
  return k2node_report_rec(input_node, row, REPORT_ROW, 0, st, result);
}

int k2node_report_column_interactively(struct k2node *input_node,
                                       unsigned long col, struct k2qstate *st,
                                       point_reporter_fun_t point_reporter,
                                       void *report_state) {
  return k2node_report_interactively_rec(input_node, col, REPORT_COLUMN, 0, st,
                                         point_reporter, report_state);
}

int k2node_report_row_interactively(struct k2node *input_node,
                                    unsigned long row, struct k2qstate *st,
                                    point_reporter_fun_t point_reporter,
                                    void *report_state) {
  return k2node_report_interactively_rec(input_node, row, REPORT_ROW, 0, st,
                                         point_reporter, report_state);
}

struct k2node *create_k2node(void) {
  return k2tree_allocate_k2node();
}

int free_rec_k2node(struct k2node *input_node, unsigned long current_depth,
                    unsigned long cut_depth) {
  if (current_depth == cut_depth) {
    free_rec_block(input_node->k2subtree.block_child);
  } else {
    for (int child_pos = 0; child_pos < 4; child_pos++) {
      struct k2node *child_node = input_node->k2subtree.children[child_pos];
      if (child_node)
        free_rec_k2node(child_node, current_depth + 1, cut_depth);
    }
  }

  k2tree_free_k2node(input_node);
  return SUCCESS_ECODE_K2T;
}

struct k2tree_measurement k2node_measure_tree_size(struct k2node *input_node,
                                                   unsigned long cut_depth) {
  return k2node_measure_tree_size_rec(input_node, 0, cut_depth);
}

int init_k2qstate(struct k2qstate *st, TREE_DEPTH_T treedepth,
                  MAX_NODE_COUNT_T max_nodes_count, TREE_DEPTH_T cut_depth) {
  CHECK_ERR(init_queries_state(&st->qs, treedepth - cut_depth, max_nodes_count,
                               NULL));
  init_morton_code(&st->mc, treedepth);
  st->cut_depth = cut_depth;
  st->k2tree_depth = treedepth;
  return SUCCESS_ECODE_K2T;
}

int clean_k2qstate(struct k2qstate *st) {
  CHECK_ERR(finish_queries_state(&st->qs));
  clean_morton_code(&st->mc);
  return SUCCESS_ECODE_K2T;
}

static inline void sip_assign_valid_part(int *valid_parts,
                                         int *selected_children, int index,
                                         struct k2node **current_nodes) {
  for (int i = 0; i < 2; i++) {
    if (valid_parts[i])
      valid_parts[i] =
          (current_nodes[index]->k2subtree.children[selected_children[i]] !=
           NULL);
  }
}

struct intermediate_reporter_data {
  coord_reporter_fun_t rep_fun;
  void *report_state;
  unsigned long base_coord;
};

static void intermediate_reporter(unsigned long coord, void *report_state) {
  struct intermediate_reporter_data *ird =
      (struct intermediate_reporter_data *)report_state;
  ird->rep_fun(coord + ird->base_coord, ird->report_state);
}

int k2node_sip_join_rec(struct k2node_sip_input input,
                        TREE_DEPTH_T current_depth, TREE_DEPTH_T treedepth,
                        TREE_DEPTH_T cut_depth, struct morton_code *mc,
                        struct k2node **current_nodes,
                        struct sip_ipoint *join_coords,
                        coord_reporter_fun_t coord_reporter,
                        void *report_state) {

  if (current_depth == cut_depth) {
    struct pair2dl pair;
    convert_morton_code_to_coordinates(mc, &pair);
    unsigned long base_coord;
    switch (join_coords[input.join_size - 1].coord_type) {
    case COLUMN_COORD:
      base_coord = pair.row;
      break;
    case ROW_COORD:
    default:
      base_coord = pair.col;
      break;
    }
    base_coord = base_coord << (treedepth - cut_depth);

    struct sip_join_input sji;
    sji.blocks = input._blocks;
    sji.qss = input._qss;
    sji.join_coords = join_coords + current_depth * input.join_size;
    sji.join_size = input.join_size;
    for (int j = 0; j < input.join_size; j++) {
      int jindex = current_depth * input.join_size + j;
      struct block *root_block = current_nodes[jindex]->k2subtree.block_child;
      sji.blocks[j] = root_block;
      sji.qss[j] = &input.sts[j]->qs;
    }

    struct intermediate_reporter_data ird;
    ird.base_coord = base_coord;
    ird.rep_fun = coord_reporter;
    ird.report_state = report_state;

    return sip_join(sji, intermediate_reporter, &ird);
  }

  unsigned long half_length = 1UL
                              << ((unsigned long)treedepth - current_depth - 1);

  int valid_parts[2] = {TRUE, TRUE};
  int selected_children[2];

  for (int j = 0; j < input.join_size; j++) {
    int jindex = input.join_size * current_depth + j;
    sip_select_child(join_coords[jindex].coord, join_coords[jindex].coord_type,
                     selected_children, half_length);

    sip_assign_valid_part(valid_parts, selected_children, jindex,
                          current_nodes);
  }

  for (int i = 0; i < 2; i++) {
    if (valid_parts[i]) {
      // Will always be the last given band
      for (int j = 0; j < input.join_size; j++) {
        int jindex = current_depth * input.join_size + j;
        int next_jindex = (current_depth + 1) * input.join_size + j;
        sip_select_child(join_coords[jindex].coord,
                         join_coords[jindex].coord_type, selected_children,
                         half_length);
        current_nodes[next_jindex] =
            current_nodes[jindex]->k2subtree.children[selected_children[i]];
        join_coords[next_jindex].coord_type = join_coords[jindex].coord_type;
        join_coords[next_jindex].coord =
            join_coords[jindex].coord % half_length;
      }
      add_element_morton_code(mc, current_depth, selected_children[i]);
      CHECK_ERR(k2node_sip_join_rec(input, current_depth + 1, treedepth,
                                    cut_depth, mc, current_nodes, join_coords,
                                    coord_reporter, report_state));
    }
  }

  return SUCCESS_ECODE_K2T;
}

int k2node_sip_join(struct k2node_sip_input input,
                    coord_reporter_fun_t coord_reporter, void *report_state) {

  TREE_DEPTH_T treedepth = input.sts[0]->k2tree_depth;
  TREE_DEPTH_T cut_depth = input.sts[0]->cut_depth;
  struct k2node **current_nodes =
      malloc(sizeof(struct k2node *) * input.join_size * (cut_depth + 1));
  memcpy(current_nodes, input.nodes, sizeof(struct k2node *) * input.join_size);

  struct morton_code mc;

  init_morton_code(&mc, cut_depth);

  struct sip_ipoint *join_coords =
      malloc(sizeof(struct sip_ipoint) * input.join_size * (cut_depth + 1));
  memcpy(join_coords, input.join_coords,
         sizeof(struct sip_ipoint) * input.join_size);

  input._qss = (struct queries_state **)malloc(sizeof(struct queries_state *) *
                                               input.join_size);
  input._blocks =
      (struct block **)malloc(sizeof(struct block *) * input.join_size);

  CHECK_ERR(k2node_sip_join_rec(input, 0, treedepth, cut_depth, &mc,
                                current_nodes, join_coords, coord_reporter,
                                report_state));

  clean_morton_code(&mc);
  free(input._qss);
  free(input._blocks);
  free(current_nodes);
  free(join_coords);
  return SUCCESS_ECODE_K2T;
}

int debug_validate_k2node(struct k2node *input_node, struct k2qstate *st,
                          TREE_DEPTH_T current_depth) {
  if (current_depth == st->cut_depth) {
    return input_node->k2subtree.block_child ? 0 : 1;
  }

  for (int i = 0; i < 4; i++) {
    if (input_node->k2subtree.children[i])
      return 0;
  }

  return 1;
}

int debug_validate_k2node_rec(struct k2node *input_node, struct k2qstate *st,
                              TREE_DEPTH_T current_depth) {
  if (debug_validate_k2node(input_node, st, current_depth)) {
    return 1;
  }

  if (current_depth == st->cut_depth) {
    int result = debug_validate_block_rec(input_node->k2subtree.block_child);
    return result;
  }

  for (int i = 0; i < 4; i++) {
    if (input_node->k2subtree.children[i]) {
      int result = debug_validate_k2node_rec(input_node->k2subtree.children[i],
                                             st, current_depth + 1);
      if (result)
        return result + 1;
    }
  }

  return 0;
}
int k2node_naive_scan_points_lazy_init(
    struct k2node *input_node, struct k2qstate *st,
    struct k2node_lazy_handler_naive_scan_t *lazy_handler) {
  lazy_handler->st = st;
  lazy_handler->has_next = FALSE;
  init_k2node_lazy_naive_state_stack(&lazy_handler->states_stack,
                                     st->cut_depth * 4);
  init_lazy_naive_state_stack(&lazy_handler->sub_handler.states_stack,
                              (st->k2tree_depth - st->cut_depth) * 4);
  lazy_handler->sub_handler.qs = &st->qs;
  lazy_handler->sub_handler.has_next = FALSE;
  lazy_handler->at_leaf = FALSE;
  lazy_handler->tree_root = input_node;

  k2node_lazy_naive_state first_state;
  first_state.current_depth = 0;
  first_state.input_node = input_node;
  first_state.last_iteration = 0;
  push_k2node_lazy_naive_state_stack(&lazy_handler->states_stack, first_state);
  k2node_naive_scan_points_lazy_next(lazy_handler, &lazy_handler->next_result);

  return SUCCESS_ECODE_K2T;
}

int k2node_naive_scan_points_lazy_reset(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler) {
  lazy_handler->has_next = FALSE;
  lazy_handler->at_leaf = FALSE;

  lazy_handler->sub_handler.has_next = FALSE;

  reset_lazy_naive_state_stack(&lazy_handler->sub_handler.states_stack);
  reset_k2node_lazy_naive_state_stack(&lazy_handler->states_stack);

  k2node_lazy_naive_state first_state;
  first_state.current_depth = 0;
  first_state.input_node = lazy_handler->tree_root;
  first_state.last_iteration = 0;
  push_k2node_lazy_naive_state_stack(&lazy_handler->states_stack, first_state);
  k2node_naive_scan_points_lazy_next(lazy_handler, &lazy_handler->next_result);

  return SUCCESS_ECODE_K2T;
}

int k2node_naive_scan_points_lazy_clean(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler) {
  free_k2node_lazy_naive_state_stack(&lazy_handler->states_stack);
  free_lazy_naive_state_stack(&lazy_handler->sub_handler.states_stack);
  return SUCCESS_ECODE_K2T;
}

int k2node_naive_scan_points_lazy_next(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler, pair2dl_t *result) {
  *result = lazy_handler->next_result;
  struct k2qstate *st = lazy_handler->st;
  while (!empty_k2node_lazy_naive_state_stack(&lazy_handler->states_stack)) {
    k2node_lazy_naive_state current_state =
        pop_k2node_lazy_naive_state_stack(&lazy_handler->states_stack);
    unsigned long current_depth = current_state.current_depth;
    struct k2node *node = current_state.input_node;

    if (current_depth == st->cut_depth) {
      if (!lazy_handler->at_leaf) {
        struct pair2dl high_level_coordinates;
        convert_morton_code_to_coordinates_select_treedepth(
            &st->mc, &high_level_coordinates, st->cut_depth);
        lazy_handler->base_col = high_level_coordinates.col
                                 << (st->k2tree_depth - st->cut_depth);
        lazy_handler->base_row = high_level_coordinates.row
                                 << (st->k2tree_depth - st->cut_depth);

        lazy_handler->sub_handler.has_next = FALSE;

        lazy_naive_state first_state;
        first_state.block_depth = 0;
        first_state.input_block = node->k2subtree.block_child;
        first_state.last_iteration = 0;
        clean_child_result(&first_state.cr);

        push_lazy_naive_state_stack(&lazy_handler->sub_handler.states_stack,
                                    first_state);
        naive_scan_points_lazy_next(&lazy_handler->sub_handler,
                                    &lazy_handler->sub_handler.next_result);
        lazy_handler->at_leaf = TRUE;
      }

      int sub_has_next;
      naive_scan_points_lazy_has_next(&lazy_handler->sub_handler,
                                      &sub_has_next);
      if (!sub_has_next) {
        lazy_handler->at_leaf = FALSE;
      } else {
        naive_scan_points_lazy_next(&lazy_handler->sub_handler,
                                    &lazy_handler->next_result);
        lazy_handler->next_result.col += (long)lazy_handler->base_col;
        lazy_handler->next_result.row += (long)lazy_handler->base_row;
        lazy_handler->has_next = TRUE;
        push_k2node_lazy_naive_state_stack(&lazy_handler->states_stack,
                                           current_state);
        return SUCCESS_ECODE_K2T;
      }
      continue;
    }

    for (uint32_t child_index = current_state.last_iteration; child_index < 4;
         child_index++) {
      if (node->k2subtree.children[child_index] != NULL) {
        add_element_morton_code(&st->mc, current_depth, child_index);

        if (child_index < 3) {
          k2node_lazy_naive_state sibling_state;
          sibling_state.last_iteration = child_index + 1;
          sibling_state.input_node = current_state.input_node;
          sibling_state.current_depth = current_state.current_depth;
          push_k2node_lazy_naive_state_stack(&lazy_handler->states_stack,
                                             sibling_state);
        }

        k2node_lazy_naive_state child_state;
        child_state.last_iteration = 0;
        child_state.input_node = node->k2subtree.children[child_index];
        child_state.current_depth = current_depth + 1;
        push_k2node_lazy_naive_state_stack(&lazy_handler->states_stack,
                                           child_state);
        break;
      }
    }
  }
  lazy_handler->has_next = FALSE;
  return SUCCESS_ECODE_K2T;
}

int k2node_naive_scan_points_lazy_has_next(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler, int *result) {
  *result = lazy_handler->has_next;
  return SUCCESS_ECODE_K2T;
}

int k2node_report_band_lazy_init(
    struct k2node_lazy_handler_report_band_t *lazy_handler,
    struct k2node *input_node, struct k2qstate *st, uint64_t coord,
    int which_report);

int k2node_report_band_lazy_init(
    struct k2node_lazy_handler_report_band_t *lazy_handler,
    struct k2node *input_node, struct k2qstate *st, uint64_t coord,
    int which_report) {
  lazy_handler->st = st;
  lazy_handler->has_next = FALSE;
  init_k2node_lazy_report_band_state_t_stack(&lazy_handler->stack,
                                             st->cut_depth * 4);
  init_lazy_report_band_state_t_stack(&lazy_handler->sub_handler.stack,
                                      (st->k2tree_depth - st->cut_depth) * 4);

  lazy_handler->sub_handler.qs = &st->qs;
  lazy_handler->sub_handler.has_next = FALSE;
  lazy_handler->at_leaf = FALSE;
  lazy_handler->which_report = which_report;
  lazy_handler->tree_root = input_node;
  lazy_handler->coord_report = coord;

  k2node_lazy_report_band_state_t first_state;

  first_state.current_depth = 0;
  first_state.input_node = input_node;
  first_state.last_iteration = 0;
  first_state.current_coord = coord;
  push_k2node_lazy_report_band_state_t_stack(&lazy_handler->stack, first_state);
  k2node_report_band_next(lazy_handler, &lazy_handler->next_result);

  return SUCCESS_ECODE_K2T;
}

int k2node_report_row_lazy_init(
    struct k2node_lazy_handler_report_band_t *lazy_handler,
    struct k2node *input_node, struct k2qstate *st, uint64_t coord) {
  return k2node_report_band_lazy_init(lazy_handler, input_node, st, coord,
                                      ROW_COORD);
}

int k2node_report_column_lazy_init(
    struct k2node_lazy_handler_report_band_t *lazy_handler,
    struct k2node *input_node, struct k2qstate *st, uint64_t coord) {
  return k2node_report_band_lazy_init(lazy_handler, input_node, st, coord,
                                      COLUMN_COORD);
}

int k2node_report_band_lazy_clean(
    struct k2node_lazy_handler_report_band_t *lazy_handler) {
  report_band_lazy_clean(&lazy_handler->sub_handler);
  free_k2node_lazy_report_band_state_t_stack(&lazy_handler->stack);
  return SUCCESS_ECODE_K2T;
}

int k2node_report_band_reset(
    struct k2node_lazy_handler_report_band_t *lazy_handler) {
  lazy_handler->has_next = FALSE;
  lazy_handler->sub_handler.has_next = FALSE;
  lazy_handler->at_leaf = FALSE;
  reset_k2node_lazy_report_band_state_t_stack(&lazy_handler->stack);
  reset_lazy_report_band_state_t_stack(&lazy_handler->sub_handler.stack);

  k2node_lazy_report_band_state_t first_state;

  first_state.current_depth = 0;
  first_state.input_node = lazy_handler->tree_root;
  first_state.last_iteration = 0;
  first_state.current_coord = lazy_handler->coord_report;
  push_k2node_lazy_report_band_state_t_stack(&lazy_handler->stack, first_state);
  k2node_report_band_next(lazy_handler, &lazy_handler->next_result);

  return SUCCESS_ECODE_K2T;
}

int k2node_report_band_next(
    struct k2node_lazy_handler_report_band_t *lazy_handler, uint64_t *result) {
  *result = lazy_handler->next_result;
  struct k2qstate *st = lazy_handler->st;
  while (!empty_k2node_lazy_report_band_state_t_stack(&lazy_handler->stack)) {
    k2node_lazy_report_band_state_t current_state =
        pop_k2node_lazy_report_band_state_t_stack(&lazy_handler->stack);

    unsigned long current_depth = current_state.current_depth;
    unsigned long remaining_depth = st->k2tree_depth - current_depth;
    struct k2node *node = current_state.input_node;

    if (current_depth == st->cut_depth) {

      if (!lazy_handler->at_leaf) {
        struct pair2dl high_level_coordinates;
        convert_morton_code_to_coordinates_select_treedepth(
            &st->mc, &high_level_coordinates, st->cut_depth);

        lazy_handler->base_col = high_level_coordinates.col
                                 << (st->k2tree_depth - st->cut_depth);
        lazy_handler->base_row = high_level_coordinates.row
                                 << (st->k2tree_depth - st->cut_depth);

        lazy_handler->sub_handler.has_next = FALSE;
        lazy_handler->sub_handler.which_report = lazy_handler->which_report;
        lazy_handler->sub_handler.qs = &lazy_handler->st->qs;

        lazy_report_band_state_t first_state;
        first_state.current_coord = current_state.current_coord;
        clean_child_result(&first_state.current_cr);
        first_state.last_iteration = 0;
        first_state.current_cr.resulting_block = node->k2subtree.block_child;

        push_lazy_report_band_state_t_stack(&lazy_handler->sub_handler.stack,
                                            first_state);
        report_band_next(&lazy_handler->sub_handler,
                         &lazy_handler->sub_handler.next_result);
        lazy_handler->at_leaf = TRUE;
      }

      int sub_has_next;
      report_band_has_next(&lazy_handler->sub_handler, &sub_has_next);
      if (!sub_has_next) {
        lazy_handler->at_leaf = FALSE;
      } else {
        report_band_next(&lazy_handler->sub_handler,
                         &lazy_handler->next_result);

        if (lazy_handler->which_report == REPORT_COLUMN) {
          lazy_handler->next_result += lazy_handler->base_row;
        } else {
          lazy_handler->next_result += lazy_handler->base_col;
        }

        lazy_handler->has_next = TRUE;
        push_k2node_lazy_report_band_state_t_stack(&lazy_handler->stack,
                                                   current_state);
        return SUCCESS_ECODE_K2T;
      }
      continue;
    }

    unsigned long next_remaining_depth = remaining_depth - 1;
    unsigned long half_level = 1 << next_remaining_depth;

    for (uint32_t child_pos = current_state.last_iteration; child_pos < 4;
         child_pos++) {
      if (!REPORT_CONTINUE_CONDITION(current_state.current_coord, half_level,
                                     lazy_handler->which_report, child_pos) ||
          !node->k2subtree.children[child_pos])
        continue;
      add_element_morton_code(&st->mc, current_depth, child_pos);

      if (child_pos < 3) {
        k2node_lazy_report_band_state_t sibling_state;
        sibling_state.current_coord = current_state.current_coord;
        sibling_state.last_iteration = child_pos + 1;
        sibling_state.current_depth = current_state.current_depth;
        sibling_state.input_node = current_state.input_node;
        push_k2node_lazy_report_band_state_t_stack(&lazy_handler->stack,
                                                   sibling_state);
      }
      k2node_lazy_report_band_state_t child_state;
      child_state.current_coord = current_state.current_coord % half_level;
      child_state.last_iteration = 0;
      child_state.current_depth = current_state.current_depth + 1;
      child_state.input_node = node->k2subtree.children[child_pos];
      push_k2node_lazy_report_band_state_t_stack(&lazy_handler->stack,
                                                 child_state);
      break;
    }
  }
  lazy_handler->has_next = FALSE;
  return SUCCESS_ECODE_K2T;
}

int k2node_report_band_has_next(
    struct k2node_lazy_handler_report_band_t *lazy_handler, int *result) {
  *result = lazy_handler->has_next;
  return SUCCESS_ECODE_K2T;
}

int k2node_delete_point(struct k2node *input_node, unsigned long col,
                        unsigned long row, struct k2qstate *st,
                        int *already_not_exists) {

  *already_not_exists = FALSE;
  convert_coordinates_to_morton_code(col, row, st->k2tree_depth, &st->mc);
  int has_children = TRUE;
  return k2node_delete_point_rec(input_node, st, col, row, 0,
                                 already_not_exists, &has_children);
}

static int print_debug_k2node_rec(struct k2node *node, int curr_depth,
                                  struct k2qstate *st) {

  if (curr_depth == st->cut_depth) {
    if (node->k2subtree.block_child == NULL)
      return 0;
    printf(". BlOCK: \n");
    debug_print_block_rec(node->k2subtree.block_child);
    printf("\n\n");
    return 0;
  }

  for (int i = 0; i < 4; i++) {
    struct k2node *child = node->k2subtree.children[i];
    if (child != NULL) {
      printf("%d, ", i);
      print_debug_k2node_rec(child, curr_depth + 1, st);
    }
  }
  return 0;
}

int print_debug_k2node(struct k2node *node, struct k2qstate *st) {
  print_debug_k2node_rec(node, 0, st);
  return 0;
}

declare_stack_of_type(k2node_lazy_naive_state)
    declare_stack_of_type(k2node_lazy_report_band_state_t)

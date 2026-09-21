
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

struct interactive_report_data {
  point_reporter_fun_t point_reporter;
  void *report_state;
  ulong base_col;
  ulong base_row;
};

/* private prototypes */
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

void interactive_transform_points(ulong col, ulong row, void *data);
/* private implementations */

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
  ulong half_length = 1 << (remaining_depth - 1);

  return k2_find_subtree(next_node, st, col % half_length, row % half_length,
                         current_depth + 1);
}

int k2node_naive_scan_points_rec(struct k2node *node, struct k2qstate *st,
                                 ulong current_depth,
                                 struct vector_pair2dl_t *result) {
  if (current_depth == st->cut_depth) {
    struct pair2dl high_level_coordinates;
    long starting_pos = result->nof_items;
    convert_morton_code_to_coordinates_select_treedepth(
        &st->mc, &high_level_coordinates, st->cut_depth);
    CHECK_ERR(naive_scan_points(node->k2subtree.block_child, &st->qs, result));
    ulong base_col = high_level_coordinates.col
                     << (st->k2tree_depth - st->cut_depth);
    ulong base_row = high_level_coordinates.row
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

void interactive_transform_points(ulong col, ulong row, void *data) {
  struct interactive_report_data *middle_state =
      (struct interactive_report_data *)data;
  middle_state->point_reporter(middle_state->base_col + col,
                               middle_state->base_row + row,
                               middle_state->report_state);
}

int k2node_scan_points_interactively_rec(struct k2node *node,
                                         struct k2qstate *st,
                                         ulong current_depth,
                                         point_reporter_fun_t point_reporter,
                                         void *report_state) {
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

int k2node_report_rec(struct k2node *node, ulong coord, int which_report,
                      ulong current_depth, struct k2qstate *st,
                      struct vector_pair2dl_t *result) {

  ulong remaining_depth = st->k2tree_depth - current_depth;
  if (current_depth == st->cut_depth) {
    struct pair2dl high_level_coordinates;
    convert_morton_code_to_coordinates_select_treedepth(
        &st->mc, &high_level_coordinates, st->cut_depth);
    uint32_t starting_pos = result->nof_items;
    ulong base_col = high_level_coordinates.col
                     << (st->k2tree_depth - st->cut_depth);
    ulong base_row = high_level_coordinates.row
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

  ulong next_remaining_depth = remaining_depth - 1;
  ulong half_level = (next_remaining_depth << 1);

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

int k2node_report_interactively_rec(struct k2node *node, ulong coord,
                                    int which_report, ulong current_depth,
                                    struct k2qstate *st,
                                    point_reporter_fun_t point_reporter,
                                    void *report_state) {

  ulong remaining_depth = st->k2tree_depth - current_depth;
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

  ulong next_remaining_depth = remaining_depth - 1;
  ulong half_level = (next_remaining_depth << 1);

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

  uint32_t child_pos = get_code_at_morton_code(&st->mc, current_depth);

  if (from_node->k2subtree.children[child_pos]) {
    fprintf(stderr, "debug: fill_insertion_path bad case %p\n",
            (void *)from_node->k2subtree.children[child_pos]);
    exit(1);
  }

  from_node->k2subtree.children[child_pos] = create_k2node();

  uint32_t half_length = 1 << (remaining_depth - 1);

  return fill_insertion_path(from_node->k2subtree.children[child_pos],
                             col % half_length, row % half_length, st,
                             current_depth + 1);
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

int k2node_insert_point(struct k2node *root_node, ulong col, ulong row,
                        struct k2qstate *st) {
  convert_coordinates_to_morton_code(col, row, st->k2tree_depth, &st->mc);
  struct k2_find_subtree_result tr_result =
      k2_find_subtree(root_node, st, col, row, 0);
  if (!tr_result.exists) {
    tr_result = fill_insertion_path(tr_result.last_node_visited, tr_result.col,
                                    tr_result.row, st, tr_result.depth_reached);
  }
  st->qs.root = tr_result.subtree_root;
  return insert_point(tr_result.subtree_root, tr_result.col, tr_result.row,
                      &st->qs);
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

int k2node_report_row_interactively(struct k2node *input_node, ulong row,
                                    struct k2qstate *st,
                                    point_reporter_fun_t point_reporter,
                                    void *report_state) {
  return k2node_report_interactively_rec(input_node, row, REPORT_ROW, 0, st,
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


static inline void sip_select_child(long coord, coord_t coord_type,
                                    int *selected_child_1,
                                    int *selected_child_2, long half_length) {
  switch (coord_type) {
  case COLUMN_COORD:
    if (coord < half_length) {
      *selected_child_1 = 0;
      *selected_child_2 = 1;
    } else {
      *selected_child_1 = 2;
      *selected_child_2 = 3;
    }
    break;
  case ROW_COORD:
  default:
    if (coord < half_length) {
      *selected_child_1 = 0;
      *selected_child_2 = 2;
    } else {
      *selected_child_1 = 1;
      *selected_child_2 = 3;
    }
    break;
  }
}

static inline int sip_assign_valid_part(int *valid_part_1, int *valid_part_2,
                                        int selected_child_1,
                                        int selected_child_2) {
  
}

static int k2node_sip_join_rec(struct k2node_sip_input input, TREE_DEPTH_T current_depth, TREE_DEPTH_T treedepth, TREE_DEPTH_T cut_depth, struct morton_code *mc, struct vector_block_ptr_t *acc_blocks, struct k2node **current_nodes, struct sip_ipoint *join_coords){
  if(current_depth == cut_depth){

    
    // vector_block_ptr_t__insert_element(acc_blocks, current_nodes)
    return SUCCESS_ECODE_K2T;  
  }

  ulong side_length = 1UL << ((ulong)treedepth - current_depth);
  ulong half_length = side_length >> 1;

  int valid_parts[2] = {TRUE, TRUE};
  int selected_children[2];
  for(int i = 0; i < input.join_size; i++){
    sip_select_child()
  }

  

  


}

int k2node_sip_join(struct k2node_sip_input input, TREE_DEPTH_T cut_depth, TREE_DEPTH_T treedepth){
  struct k2node **current_nodes = malloc(sizeof(struct k2node *) * input.join_size);
  memcpy(current_nodes, input.nodes, sizeof(struct k2node *) * input.join_size);

  struct morton_code mc;

  init_morton_code(&mc, treedepth);

  struct vector_pair2dl_t acc_blocks;
  vector_block_ptr_t__init_vector_with_capacity(&acc_blocks, 8);

  struct sip_ipoint *join_coords = malloc(sizeof(struct sip_ipoint) * input.join_size);
  memcpy(join_coords, input.join_coords, sizeof(struct sip_ipoint) * input.join_size);

  CHECK_ERR(k2node_sip_join_rec(input, 0, treedepth, cut_depth, &mc, &acc_blocks, current_nodes, join_coords));


  vector_block_ptr_t__free_vector(&acc_blocks);
  clean_morton_code(&mc);
  free(current_nodes);
  free(join_coords);
}

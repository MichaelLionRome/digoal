
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
extern "C" {
#include <block.h>
#include <block_frontier.h>
#include <block_topology.h>
#include <definitions.h>
#include <k2node.h>
#include <queries_state.h>
}

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

#include <gtest/gtest.h>

struct k2_data {
  struct k2node *root;
  struct k2qstate st;
};

static k2_data create_k2_line(TREE_DEPTH_T tree_depth, TREE_DEPTH_T cut_depth,
                              coord_t coord_type, ulong position) {
  k2_data result;
  result.root = create_k2node();
  init_k2qstate(&result.st, tree_depth, 255, cut_depth);
  int already_exists;
  for (ulong i = 0; i < static_cast<ulong>(1 << tree_depth); i++) {
    if (coord_type == COLUMN_COORD) {
      k2node_insert_point(result.root, position, i, &result.st,
                          &already_exists);
    } else {
      k2node_insert_point(result.root, i, position, &result.st,
                          &already_exists);
    }
  }
  return result;
}

static void clean_k2_line(k2_data &data) {
  free_rec_k2node(data.root, 0, data.st.cut_depth);
  clean_k2qstate(&data.st);
}

TEST(sip_tests, can_retrieve_single_elements_bands_1) {
  const TREE_DEPTH_T tree_depth = 10;
  const TREE_DEPTH_T cut_depth = 5;

  for (int coord_type = 0; coord_type <= 1; coord_type++) {
    for (ulong line_position = 0; line_position < (1 << tree_depth);
         line_position++) {

      coord_t line_coord_type = (coord_t)coord_type;
      coord_t join_coord_type = (coord_t)(1 - coord_type);

      auto data =
          create_k2_line(tree_depth, cut_depth, line_coord_type, line_position);

      for (ulong i = 0; i < (1 << tree_depth); i++) {

        struct sip_ipoint sip_point;
        sip_point.coord = i;
        sip_point.coord_type = join_coord_type;
        struct k2qstate *st_p = &data.st;

        struct k2node_sip_input ksi;

        ksi.nodes = &data.root;
        ksi.join_size = 1;
        ksi.join_coords = &sip_point;
        ksi.sts = &st_p;

        std::set<ulong> coords;
        k2node_sip_join(
            ksi,
            [](ulong coord, void *report_state) {
              auto &data = *reinterpret_cast<std::set<ulong> *>(report_state);
              data.insert(coord);
            },
            &coords);

        ASSERT_EQ(coords.size(), 1);
        ASSERT_EQ(*coords.begin(), line_position);
      }

      ASSERT_EQ(debug_validate_k2node_rec(data.root, &data.st, 0), 0);

      clean_k2_line(data);
    }
  }
}

TEST(sip_tests, can_retrieve_band_with_single_sip) {
  struct k2node *root_node = create_k2node();

  struct k2qstate st;
  TREE_DEPTH_T treedepth = 32;
  TREE_DEPTH_T cutdepth = 10;
  init_k2qstate(&st, treedepth, 255, cutdepth);

  int already_exists;

  ulong col_choice = 1 << 30;

  for (ulong row = col_choice; row < col_choice + 1000; row++) {
    k2node_insert_point(root_node, col_choice, row, &st, &already_exists);
  }

  struct k2node_sip_input ksi;

  struct sip_ipoint sip_point;
  sip_point.coord = col_choice;
  sip_point.coord_type = COLUMN_COORD;

  struct k2qstate *st_p = &st;

  ksi.nodes = &root_node;
  ksi.join_size = 1;
  ksi.join_coords = &sip_point;
  ksi.sts = &st_p;

  std::set<ulong> coords;
  k2node_sip_join(
      ksi,
      [](ulong coord, void *report_state) {
        auto &data = *reinterpret_cast<std::set<ulong> *>(report_state);
        data.insert(coord);
      },
      &coords);

  size_t pos = 0;
  for (auto it = coords.begin(); it != coords.end(); it++, pos++) {
    ASSERT_EQ(*it, col_choice + pos);
  }

  ASSERT_EQ(debug_validate_k2node_rec(root_node, &st, 0), 0);

  free_rec_k2node(root_node, 0, cutdepth);
  clean_k2qstate(&st);
}

TEST(sip_tests, test_join_two) {
  uint32_t treedepth = 5;
  uint32_t cut_depth = 3;

  constexpr int join_size = 2;

  struct k2node *nodes[join_size];
  struct k2qstate *sts[join_size];
  for (int i = 0; i < join_size; i++) {
    nodes[i] = create_k2node();
    sts[i] = new struct k2qstate;
    init_k2qstate(sts[i], treedepth, MAX_NODES_IN_BLOCK, cut_depth);
  }

  int already_exists;
  for (int i = 0; i < 10; i++) {
    k2node_insert_point(nodes[0], i, 2, sts[0], &already_exists);
    k2node_insert_point(nodes[1], 4, i, sts[1], &already_exists);
  }

  k2node_insert_point(nodes[0], 13, 2, sts[0], &already_exists);
  k2node_insert_point(nodes[1], 4, 13, sts[1], &already_exists);

  struct k2node_sip_input ksi;

  ksi.nodes = nodes;
  ksi.join_size = join_size;
  ksi.join_coords = new struct sip_ipoint[join_size];

  ksi.join_coords[0].coord = 2;
  ksi.join_coords[0].coord_type = ROW_COORD;
  ksi.join_coords[1].coord = 4;
  ksi.join_coords[1].coord_type = COLUMN_COORD;
  ksi.sts = sts;

  std::set<ulong> coords;
  k2node_sip_join(
      ksi,
      [](ulong coord, void *report_state) {
        auto &data = *reinterpret_cast<std::set<ulong> *>(report_state);
        data.insert(coord);
      },
      &coords);

  ASSERT_EQ(coords.size(), 11);

  ASSERT_EQ(debug_validate_k2node_rec(nodes[0], sts[0], 0), 0);
  ASSERT_EQ(debug_validate_k2node_rec(nodes[1], sts[1], 0), 0);

  delete[] ksi.join_coords;
  for (int i = 0; i < join_size; i++) {
    free_rec_k2node(nodes[i], 0, cut_depth);
    clean_k2qstate(sts[i]);
    delete sts[i];
  }
}

TEST(sip_tests, test_join_four) {
  uint32_t treedepth = 5;
  uint32_t cut_depth = 3;

  constexpr int join_size = 4;

  struct k2node *nodes[join_size];
  struct k2qstate *sts[join_size];
  for (int i = 0; i < join_size; i++) {
    nodes[i] = create_k2node();
    sts[i] = new struct k2qstate;
    init_k2qstate(sts[i], treedepth, MAX_NODES_IN_BLOCK, cut_depth);
  }
  int already_exists;
  for (int i = 0; i < 10; i++) {
    k2node_insert_point(nodes[0], i, 2, sts[0], &already_exists);
    k2node_insert_point(nodes[1], 4, i, sts[1], &already_exists);
  }

  k2node_insert_point(nodes[0], 13, 2, sts[0], &already_exists);
  k2node_insert_point(nodes[1], 4, 13, sts[1], &already_exists);

  k2node_insert_point(nodes[2], 0, 7, sts[2], &already_exists);
  k2node_insert_point(nodes[2], 5, 7, sts[2], &already_exists);
  k2node_insert_point(nodes[2], 13, 7, sts[2], &already_exists);

  ulong side = 1UL << treedepth;
  for (ulong col = 0; col < side; col++) {
    for (ulong row = 0; row < side; row++) {
      k2node_insert_point(nodes[3], col, row, sts[3], &already_exists);
    }
  }

  struct k2node_sip_input ksi;

  ksi.nodes = nodes;
  ksi.join_size = join_size;
  ksi.join_coords = new struct sip_ipoint[join_size];

  ksi.join_coords[0].coord = 2;
  ksi.join_coords[0].coord_type = ROW_COORD;
  ksi.join_coords[1].coord = 4;
  ksi.join_coords[1].coord_type = COLUMN_COORD;
  ksi.join_coords[2].coord = 7;
  ksi.join_coords[2].coord_type = ROW_COORD;
  ksi.join_coords[3].coord = 23;
  ksi.join_coords[3].coord_type = COLUMN_COORD;
  ksi.sts = sts;

  std::set<ulong> coords;
  k2node_sip_join(
      ksi,
      [](ulong coord, void *report_state) {
        auto &data = *reinterpret_cast<std::set<ulong> *>(report_state);
        data.insert(coord);
      },
      &coords);

  ASSERT_EQ(coords.size(), 3);
  ASSERT_TRUE(coords.find(0) != coords.end());
  ASSERT_TRUE(coords.find(5) != coords.end());
  ASSERT_TRUE(coords.find(13) != coords.end());

  delete[] ksi.join_coords;
  for (int i = 0; i < join_size; i++) {
    free_rec_k2node(nodes[i], 0, cut_depth);
    clean_k2qstate(sts[i]);
    delete sts[i];
  }
}

TEST(sip_tests, test_join_two_exhaustive) {
  uint32_t treedepth = 5;
  uint32_t cut_depth = 3;

  constexpr int join_size = 2;

  struct k2node *nodes[join_size];
  struct k2qstate *sts[join_size];
  for (int i = 0; i < join_size; i++) {
    nodes[i] = create_k2node();
    sts[i] = new struct k2qstate;
    init_k2qstate(sts[i], treedepth, MAX_NODES_IN_BLOCK, cut_depth);
  }

  ulong side = 1UL << treedepth;
  int already_exists;
  for (ulong col = 0; col < side; col++) {
    for (ulong row = 0; row < side; row++) {
      k2node_insert_point(nodes[0], col, row, sts[0], &already_exists);
      k2node_insert_point(nodes[1], col, row, sts[1], &already_exists);
    }
  }

  struct k2node_sip_input ksi;

  ksi.nodes = nodes;
  ksi.join_size = join_size;
  ksi.join_coords = new struct sip_ipoint[join_size];
  ksi.join_coords[0].coord = 2;
  ksi.join_coords[0].coord_type = ROW_COORD;
  ksi.join_coords[1].coord = 4;
  ksi.join_coords[1].coord_type = COLUMN_COORD;
  ksi.sts = sts;

  std::vector<ulong> coords;
  k2node_sip_join(
      ksi,
      [](ulong coord, void *report_state) {
        auto &data = *reinterpret_cast<std::vector<ulong> *>(report_state);
        data.push_back(coord);
      },
      &coords);

  ASSERT_EQ(coords.size(), side);

  std::sort(coords.begin(), coords.end());
  for (size_t i = 0; i < coords.size(); i++) {
    ASSERT_EQ(coords[i], i);
  }

  delete[] ksi.join_coords;
  for (int i = 0; i < join_size; i++) {
    free_rec_k2node(nodes[i], 0, cut_depth);
    clean_k2qstate(sts[i]);
    delete sts[i];
  }
}

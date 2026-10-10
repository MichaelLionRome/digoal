
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

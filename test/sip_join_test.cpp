
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
#include <queries_state.h>
}

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

#include <gtest/gtest.h>

TEST(sip_tests, test_join_four) {
  uint32_t treedepth = 5;

  constexpr int join_size = 4;

  struct block *blocks[join_size];
  struct queries_state *qss[join_size];
  for (int i = 0; i < join_size; i++) {
    blocks[i] = create_block();
    qss[i] = new struct queries_state;
    init_queries_state(qss[i], treedepth, MAX_NODES_IN_BLOCK, blocks[i]);
  }

  for (int i = 0; i < 10; i++) {
    insert_point(blocks[0], i, 2, qss[0]);
    insert_point(blocks[1], 4, i, qss[1]);
  }

  insert_point(blocks[2], 0, 7, qss[2]);
  insert_point(blocks[2], 5, 7, qss[2]);
  insert_point(blocks[2], 13, 7, qss[2]);

  insert_point(blocks[1], 4, 13, qss[1]);
  insert_point(blocks[0], 13, 2, qss[0]);

  ulong side =  1UL << treedepth;
  for(ulong col = 0; col < side; col++){
    for(ulong row = 0; row < side; row++){
      insert_point(blocks[3], col, row, qss[3]);
    }
  }
  
  
  struct sip_join_input sji;

  sji.blocks = blocks;
  sji.join_size = join_size;
  sji.join_coords = new struct sip_ipoint[join_size];
  sji.join_coords[0].coord = 2;
  sji.join_coords[0].coord_type = ROW_COORD;
  sji.join_coords[1].coord = 4;
  sji.join_coords[1].coord_type = COLUMN_COORD;
  sji.join_coords[2].coord = 7;
  sji.join_coords[2].coord_type = ROW_COORD;
  sji.join_coords[3].coord = 23;
  sji.join_coords[3].coord_type = COLUMN_COORD;
  sji.qss = qss;

  std::set<ulong> coords;
  sip_join(
      sji,
      [](ulong coord, void *report_state) {
        auto &data = *reinterpret_cast<std::set<ulong> *>(report_state);
        data.insert(coord);
      },
      &coords);

  ASSERT_TRUE(coords.find(0) != coords.end());
  ASSERT_TRUE(coords.find(5) != coords.end());
  ASSERT_TRUE(coords.find(13) != coords.end());
  ASSERT_EQ(coords.size(), 3);

  delete[] sji.join_coords;
  for (int i = 0; i < join_size; i++) {
    free_rec_block(blocks[i]);
    finish_queries_state(qss[i]);
    delete qss[i];
  }
}

TEST(sip_tests, test_join_two_exhaustive) {
  uint32_t treedepth = 5;

  constexpr int join_size = 2;

  struct block *blocks[join_size];
  struct queries_state *qss[join_size];
  for (int i = 0; i < join_size; i++) {
    blocks[i] = create_block();
    qss[i] = new struct queries_state;
    init_queries_state(qss[i], treedepth, MAX_NODES_IN_BLOCK, blocks[i]);
  }

  ulong side =  1UL << treedepth;
  for(ulong col = 0; col < side; col++){
    for(ulong row = 0; row < side; row++){
      insert_point(blocks[0], col, row, qss[0]);
      insert_point(blocks[1], col, row, qss[1]);
    }
  }
  
  
  struct sip_join_input sji;

  sji.blocks = blocks;
  sji.join_size = join_size;
  sji.join_coords = new struct sip_ipoint[join_size];
  sji.join_coords[0].coord = 2;
  sji.join_coords[0].coord_type = ROW_COORD;
  sji.join_coords[1].coord = 4;
  sji.join_coords[1].coord_type = COLUMN_COORD;
  sji.qss = qss;

  std::vector<ulong> coords;
  sip_join(
      sji,
      [](ulong coord, void *report_state) {
        auto &data = *reinterpret_cast<std::vector<ulong> *>(report_state);
        data.push_back(coord);
      },
      &coords);

  
  ASSERT_EQ(coords.size(), side);

  std::sort(coords.begin(), coords.end());
  for(size_t i = 0; i < coords.size(); i++){
    ASSERT_EQ(coords[i], i);
  }

  delete[] sji.join_coords;
  for (int i = 0; i < join_size; i++) {
    free_rec_block(blocks[i]);
    finish_queries_state(qss[i]);
    delete qss[i];
  }
}

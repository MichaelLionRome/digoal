//
// Created by cristobal on 27-08-21.
//
/*
MIT License

Copyright (c) 2021 Cristobal Miranda T.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
                                                              copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
            */
#include <gtest/gtest.h>

extern "C" {
#include <block.h>
#include <block_frontier.h>
#include <block_topology.h>
#include <queries_state.h>
}

#include "block_wrapper.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

using namespace std;

#define _SAFE_OP_K2(op)                                                        \
  do {                                                                         \
    if ((op) != SUCCESS_ECODE_K2T) {                                           \
      printf("There was an error while running %s. Error code: %d\n", (#op),   \
             (op));                                                            \
      exit(op);                                                                \
    }                                                                          \
  } while (0)

TEST(block_delete_test, simple_delete) {
  uint32_t tree_depth = 5;
  struct block *root_block = create_block();
  struct queries_state qs {};
  init_queries_state(&qs, tree_depth, 8, root_block);

  std::vector<std::pair<unsigned long, unsigned long>> data = {
      {1, 1},   {8, 8},   {4, 14},  {14, 14}, {14, 17},
      {14, 18}, {21, 18}, {21, 27}, {25, 27},
  };

  int already_exists;
  for (const auto &p : data) {
    _SAFE_OP_K2(
        insert_point(root_block, p.first, p.second, &qs, &already_exists));

    // debug_print_block_rec(root_block);
  }

  int was_deleted;

  int i = 0;
  for (const auto &p : data) {
    _SAFE_OP_K2(delete_point(root_block, p.first, p.second, &qs, &was_deleted));
    int is_valid = debug_validate_block_rec(root_block) == 0;
    ASSERT_TRUE(is_valid) << "failed at i = " << i;
    i++;
    // if(i==3) break;
  }

  for (const auto &p : data) {
    int does_have_point;
    _SAFE_OP_K2(
        has_point(root_block, p.first, p.second, &qs, &does_have_point));
    ASSERT_FALSE(does_have_point)
        << "didnt delete point (" << p.first << "," << p.second << ")";
  }

  finish_queries_state(&qs);
  free_rec_block(root_block);
  // k2tree_free_block(root_block);
}

TEST(block_delete_test, delete_full_1) {
  uint32_t tree_depth = 8;
  struct block *root_block = create_block();
  struct queries_state qs {};
  init_queries_state(&qs, tree_depth, 256, root_block);

  int already_exists;
  int side = 1 << (tree_depth - 1);
  for (int i = 0; i < side; i++) {
    for (int j = 0; j < side; j++) {
      _SAFE_OP_K2(insert_point(root_block, i, j, &qs, &already_exists));
    }
  }

  int was_deleted;

  for (int i = 0; i < side; i++) {
    for (int j = 0; j < side; j++) {
      _SAFE_OP_K2(delete_point(root_block, i, j, &qs, &was_deleted));
      int is_valid = debug_validate_block_rec(root_block) == 0;
      ASSERT_TRUE(is_valid) << "failed at i = " << i;
      int does_have_point;
      _SAFE_OP_K2(has_point(root_block, i, j, &qs, &does_have_point));
      ASSERT_FALSE(does_have_point)
          << "didnt delete point (" << i << "," << j << ")";
    }
  }

  for (int i = 0; i < side; i++) {
    for (int j = 0; j < side; j++) {
      int does_have_point;
      _SAFE_OP_K2(has_point(root_block, i, j, &qs, &does_have_point));
      ASSERT_FALSE(does_have_point)
          << "didnt delete point (" << i << "," << j << ")";
    }
  }

  finish_queries_state(&qs);
  free_rec_block(root_block);
}

TEST(block_delete_test, delete_full_2) {
  uint32_t tree_depth = 10;
  struct block *root_block = create_block();
  struct queries_state qs {};
  init_queries_state(&qs, tree_depth, 256, root_block);

  int already_exists;
  int side = 1 << (tree_depth - 1);

  for (int i = 0; i < side; i++) {
    for (int j = 0; j < side; j++) {
      _SAFE_OP_K2(insert_point(root_block, i, j, &qs, &already_exists));
    }
  }

  int was_deleted;

  for (int i = 0; i < side; i++) {
    for (int j = 0; j < side; j++) {
      _SAFE_OP_K2(delete_point(root_block, i, j, &qs, &was_deleted));
    }
  }

  for (int i = 0; i < side; i++) {
    for (int j = 0; j < side; j++) {
      int does_have_point;
      _SAFE_OP_K2(has_point(root_block, i, j, &qs, &does_have_point));
      ASSERT_FALSE(does_have_point)
          << "didnt delete point (" << i << "," << j << ")";
    }
  }

  finish_queries_state(&qs);
  free_rec_block(root_block);
  // k2tree_free_block(root_block);
}

TEST(block_delete_test, ignore_non_existent_test_1) {
  int tree_depth = 3;
  unsigned long side = 1u << tree_depth;
  // unsigned long matrix_size = side * side;
  BlockWrapper b(tree_depth, 16);
  for (unsigned long i = 0; i < side; i++)
    for (unsigned long j = 0; j < side; j++) {
      b.insert(i, j);
    }

  std::string prev_rep;
  // auto prev_rep = b.getStringRep();
  for (int k = 0; k < 100; k++) {
    b.erase(5, 5);
    auto curr_rep = b.getStringRep();
    if (k > 0) {
      ASSERT_EQ(curr_rep, prev_rep) << "failed at k = " << k;
    }
    prev_rep = curr_rep;

    for (unsigned long i = 0; i < side; i++)
      for (unsigned long j = 0; j < side; j++) {
        ASSERT_TRUE((i == 5 && j == 5) || b.has(i, j))
            << "(" << k << ") failed at i = " << i << ", j = " << j;
      }
  }
}

TEST(block_delete_test, problematic_1) {
  int tree_depth = 3;
  unsigned long side = 1u << tree_depth;
  unsigned long matrix_size = side * side;
  BlockWrapper b(tree_depth, 16);

  unsigned long ins_sz = matrix_size / 2 + 8;
  unsigned long er_sz = ins_sz - 3;

  for (unsigned long i = 0; i < ins_sz; i++) {
    unsigned long col = i / side;
    unsigned long row = i % side;
    b.insert(col, row);
  }

  for (unsigned long i = 0; i < er_sz; i++) {
    unsigned long col = i / side;
    unsigned long row = i % side;

    b.erase(col, row);
    bool exists = b.has(col, row);

    ASSERT_FALSE(exists) << "(depth = " << tree_depth << ")"
                         << "has " << col << ", " << row
                         << " but should have been deleted... (1)";
  }

  for (unsigned long i = 0; i < er_sz; i++) {
    unsigned long col = i / side;
    unsigned long row = i % side;
    bool exists = b.has(col, row);

    ASSERT_FALSE(exists) << "(depth = " << tree_depth << ")"
                         << "has " << col << ", " << row
                         << " but should have been deleted..., i = " << i;
  }
}

TEST(block_delete_test, random_test_1) {
  auto gen_col = [](const std::vector<unsigned long> &ids, size_t i) {
    return ids[i];
  };
  auto gen_row = [](const std::vector<unsigned long> &ids, size_t i) {
    return (ids[i] + ids.size() * 33) % ids.size();
  };
  for (uint32_t treedepth = 3; treedepth <= 6; treedepth++) {
    for (int seed = 0; seed < 1000; seed++) {
      srand(seed);
      unsigned long side = 1u << treedepth;
      unsigned long matrix_size = side * side;
      BlockWrapper b(treedepth, 16);

      std::vector<unsigned long> ids(matrix_size, 0);

      for (unsigned long i = 0; i < matrix_size; i++)
        ids[i] = i;

      random_shuffle(ids.begin(), ids.end());

      for (unsigned long i = 0; i < matrix_size; i++) {
        unsigned long col = gen_col(ids, i);
        unsigned long row = gen_row(ids, i);
        b.insert(col, row);
      }

      for (unsigned long i = 0; i < matrix_size; i++) {
        unsigned long col = gen_col(ids, i);
        unsigned long row = gen_row(ids, i);
        b.erase(col, row);
      }
      for (unsigned long i = 0; i < matrix_size; i++) {
        unsigned long col = gen_col(ids, i);
        unsigned long row = gen_row(ids, i);
        bool exists = b.has(col, row);
        if (exists) {
        }
        ASSERT_FALSE(exists) << "(depth = " << treedepth << ")"
                             << "has " << col << ", " << row
                             << " but should have been deleted..."
                             << ", seed=" << seed;
      }
      ASSERT_EQ(debug_validate_block_rec(b.b), 0);
    }
  }
}

TEST(block_delete_test, diagonal_test_1) {
  for (uint32_t treedepth = 3; treedepth <= 17; treedepth++) {
    unsigned long side = std::min(
        (unsigned long)(1UL << (unsigned long)treedepth), (unsigned long)1e9);
    BlockWrapper b(treedepth, treedepth * 2);
    for (unsigned long i = 0; i < side; i++) {
      b.insert(i, i);
    }
    for (unsigned long i = 0; i < side; i++) {
      b.erase(i, i);
      if (treedepth < 17) {
        ASSERT_EQ(debug_validate_block_rec(b.b), 0);
      }
      ASSERT_FALSE(b.has(i, i));
    }
  }
}

TEST(block_delete_test, can_erase_without_altering_others_1) {
  unsigned long treedepth = 4;
  unsigned long side = (unsigned long)(1UL << treedepth);
  for (unsigned long i = 0; i < side; i++) {
    for (unsigned long j = 0; j < side; j++) {
      BlockWrapper bv(treedepth);

      for (unsigned long k = 0; k < side; k++) {
        for (unsigned long l = 0; l < side; l++) {
          bv.insert(k, l);
        }
      }
      for (int u = 0; u < 3; u++)
        bv.erase(i, j);
      for (unsigned long k = 0; k < side; k++) {
        for (unsigned long l = 0; l < side; l++) {
          if (k == i && l == j)
            continue;
          ASSERT_TRUE(bv.has(k, l))
              << "failed at i=" << i << ", j=" << j << "k=" << k << "l=" << l;
        }
      }
    }
  }
}

TEST(test_copy_nodes, test_1) {
  BlockWrapper bv(128);
  bv.init_topology_debug(256);
  bv.bitset(0);

  bv.bitset(64);
  BlockWrapper bv2(128);
  bv2.init_topology_debug(256);

  copy_nodes_between_blocks_uarr(
      bv.get_root()->container, bv.get_root()->container_size,
      bv2.get_root()->container, bv2.get_root()->container_size, 0, 32, 17);

  ASSERT_TRUE(bv2.bitread(128));
  ASSERT_TRUE(bv2.bitread(192));
}

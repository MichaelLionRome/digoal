#include <gtest/gtest.h>

#include "block_wrapper.hpp"

#include <iostream>
#include <string>

using namespace std;

TEST(some_bitvector_test_s, test1) {
  BlockWrapper some_bitvector(8, 1000);
  some_bitvector.init_topology_debug(256);

  some_bitvector.bitset(0);
  some_bitvector.bitset(1);
  some_bitvector.bitset(2);

  bool bitread_0 = some_bitvector.bitread(0);

  ASSERT_TRUE(bitread_0) << "0 failed";
  ASSERT_TRUE(some_bitvector.bitread(1)) << "1 failed";
  ASSERT_TRUE(some_bitvector.bitread(2)) << "2 failed";
  ASSERT_FALSE(some_bitvector.bitread(3)) << "3 failed";
  ASSERT_FALSE(some_bitvector.bitread(4)) << "4 failed";
  ASSERT_FALSE(some_bitvector.bitread(5)) << "5 failed";
  ASSERT_FALSE(some_bitvector.bitread(6)) << "6 failed";
  ASSERT_FALSE(some_bitvector.bitread(7)) << "7 failed";
}

TEST(fills1000, test1) {
  BlockWrapper some_bitvector(32, 128);
  some_bitvector.init_topology_debug(256);

  for (int i = 0; i < 10; i++) {
    some_bitvector.bitset(i);
  }

  for (int i = 0; i < 10; i++) {
    ASSERT_TRUE(some_bitvector.bitread(i));
  }

  for (int i = 10; i < 1000; i++) {
    ASSERT_FALSE(some_bitvector.bitread(i));
  }
}

TEST(borders_test, test1) {

  string input_ =
      "11111111111111111111111111111111111111111111110011111111110011111111";
  BlockWrapper some_bitvector(input_.size());
  some_bitvector.init_topology_debug(256);

  for (int i = 0; i < (int)some_bitvector.bv_size(); i++) {
    if (input_[i] == '1')
      some_bitvector.bitset(i);
  }

  uint32_t portion = some_bitvector.bitsread(56, 67);
  ASSERT_EQ(3327, portion);
}

TEST(set_value, test1) {
  BlockWrapper bv(128);
  bv.init_topology_debug(256);
  bv.bitset(123);
  ASSERT_TRUE(bv.bitread(123));
  for (int i = 0; i < 128; i++) {
    if (i != 123) {
      ASSERT_FALSE(bv.bitread(i));
    }
  }
}

TEST(set_value, test2) {
  BlockWrapper bv(128);
  bv.init_topology_debug(256);
  for (int i = 0; i < 128; i++) {
    bv.bitset(i);
  }
  for (int i = 0; i < 128; i++) {
    ASSERT_TRUE(bv.bitread(i));
  }
}

TEST(set_value, test3) {
  BlockWrapper bv(128);
  bv.init_topology_debug(256);
  bv.bitset(0);
  uint32_t left_most_block = bv.read_block(0);
  ASSERT_EQ(1u << 31u, left_most_block);
}

TEST(set_value, test4) {
  BlockWrapper bv(128);
  bv.init_topology_debug(256);
  bv.bitset(31);
  uint32_t left_most_block = bv.read_block(0);
  ASSERT_EQ(1, left_most_block);
}

TEST(clear_value, test1) {
  BlockWrapper bv(128);
  bv.init_topology_debug(256);
  for (int i = 0; i < 128; i++) {
    bv.bitset(i);
  }

  for (int i = 0; i < 128; i++) {
    bv.bitclear(i);
  }
  for (int i = 0; i < 128; i++) {
    ASSERT_FALSE(bv.bitread(i));
  }
}

TEST(writeT, test1) {
  BlockWrapper bv(64);
  bv.init_topology_debug(256);
  uint32_t to_write = 2;
  bv.bitswrite(31, 32, to_write);
  uint32_t read_result = bv.bitsread(31, 32);
  ASSERT_EQ(2, read_result);
}

TEST(writeT, test2) {
  BlockWrapper bv(127);
  bv.init_topology_debug(256);

  uint32_t to_write = (1 << 31) + (1 << 30);

  bv.bitswrite(32, 63, to_write);
  uint32_t read_value = bv.bitsread(32, 63);
  ASSERT_EQ(to_write, read_value);
}

TEST(write_bv_to_bv, test1) {
  for (int j = 0; j < 1024 - 100; j++)
    for (int i = 0; i < 1024 - 100; i++) {

      BlockWrapper bv(127, 8192);
      bv.init_topology_debug(4096);

      uint32_t to_write = (uint32_t)-1;

      bv.bitswrite(32 + i, 63 + i, to_write);
      bv.bitswrite(64 + i, 95 + i, to_write);

      BlockWrapper bv2(127, 8192);
      bv2.init_topology_debug(4096);

      auto *root1 = bv.get_root();
      auto *root2 = bv2.get_root();

      ASSERT_EQ(bits_write_bv(root1, root2, 32 + i, 0 + j, 64), 0);

      uint32_t rd1;
      uint32_t rd2;
      bits_read(root2, 0 + j, 31 + j, &rd1);
      bits_read(root2, 32 + j, 63 + j, &rd2);
      ASSERT_EQ((int)rd1, -1);
      ASSERT_EQ((int)rd2, -1);
    }
}
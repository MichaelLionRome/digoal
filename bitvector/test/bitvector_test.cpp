#include <gtest/gtest.h>

#include "bitvector_wrapper.h"

#include <iostream>
#include <string>

using namespace std;

TEST(some_bitvector_test_s, test1) {
  BitvectorWrapper some_bitvector(8);

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

TEST(fills200x4, test1) {
  BitvectorWrapper some_bitvector(200);

  for (int i = 0; i < 10; i++) {
    some_bitvector.bitset(i);
  }

  for (int i = 0; i < 10; i++) {
    ASSERT_TRUE(some_bitvector.bitread(i)) << "Bitread i = " << i << " failed";
  }

  for (int i = 10; i < 200 * 4; i++) {
    ASSERT_FALSE(some_bitvector.bitread(i)) << "Bitread i = " << i << " failed";
  }
}

TEST(borders_test, test1) {

  string input_ =
      "11111111111111111111111111111111111111111111110011111111110011111111";
  BitvectorWrapper some_bitvector(input_.size());
  for (int i = 0; i < (int)some_bitvector.size(); i++) {
    if (input_[i] == '1')
      some_bitvector.bitset(i);
  }

  uint32_t portion = some_bitvector.bitsread(56, 67);
  ASSERT_EQ(3327, portion);
}

TEST(creation, test1) {
  BitvectorWrapper some_bitvector(5);
  ASSERT_EQ(5, some_bitvector.size());
}

TEST(set_value, test1) {
  BitvectorWrapper bv(128);
  bv.bitset(123);
  ASSERT_TRUE(bv.bitread(123));
  for (int i = 0; i < 128; i++) {
    if (i != 123) {
      ASSERT_FALSE(bv.bitread(i));
    }
  }
}

TEST(set_value, test2) {
  BitvectorWrapper bv(128);
  for (int i = 0; i < 128; i++) {
    bv.bitset(i);
  }
  for (int i = 0; i < 128; i++) {
    ASSERT_TRUE(bv.bitread(i));
  }
}

TEST(set_value, test3) {
  BitvectorWrapper bv(128);
  bv.bitset(0);
  uint32_t left_most_block = bv.read_block(0);
  ASSERT_EQ(1u << 31u, left_most_block);
}

TEST(set_value, test4) {
  BitvectorWrapper bv(128);
  bv.bitset(31);
  uint32_t left_most_block = bv.read_block(0);
  ASSERT_EQ(1, left_most_block);
}

TEST(clear_value, test1) {
  BitvectorWrapper bv(128);
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
  BitvectorWrapper bv(64);
  uint32_t to_write = 2;
  bv.bitswrite(31, 32, to_write);
  uint32_t read_result = bv.bitsread(31, 32);
  ASSERT_EQ(2, read_result);
}

TEST(writeT, test2) {
  BitvectorWrapper bv(127);

  uint32_t to_write = (1 << 31) + (1 << 30);

  bv.bitswrite(32, 63, to_write);
  uint32_t read_value = bv.bitsread(32, 63);
  ASSERT_EQ(to_write, read_value);
}

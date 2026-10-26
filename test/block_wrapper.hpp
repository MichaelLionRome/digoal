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

#ifndef K2TREEELEMENTSTEST_BLOCK_WRAPPER_HPP
#define K2TREEELEMENTSTEST_BLOCK_WRAPPER_HPP

extern "C" {
#include <bitvector.h>
#include <block.h>
#include <block_frontier.h>
#include <block_topology.h>
#include <queries_state.h>
}

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

class BlockWrapper {

public:
  struct block *b;
  struct queries_state qs;

  BlockWrapper(uint32_t tree_depth) {
    b = create_block();
    init_queries_state(&qs, tree_depth, MAX_NODES_IN_BLOCK, b);
  }
  BlockWrapper(uint32_t tree_depth, uint32_t max_node_count)
      : BlockWrapper(tree_depth) {
    qs.max_nodes_count = max_node_count;
  }

  ~BlockWrapper() noexcept(false) {
    int err_check = free_rec_block(b);
    if (err_check)
      throw runtime_error(
          "free_rec_block: ERROR WHILE FREEING MEMORY, CODE = " +
          to_string(err_check));
    err_check = finish_queries_state(&qs);
    if (err_check)
      throw runtime_error(
          "finish_queries_state: ERROR WHILE FREEING MEMORY, CODE = " +
          to_string(err_check));
  }

  void insert(unsigned long col, unsigned long row) {
    int already_exists;
    int err_check = insert_point(b, col, row, &qs, &already_exists);
    if (err_check)
      throw runtime_error("CANT INSERT, ERROR CODE= " + to_string(err_check));
  }

  bool has(unsigned long col, unsigned long row) {
    int result;
    has_point(b, col, row, &qs, &result);
    return result == 1;
  }

  static void pass_to_ss_bin(uint32_t input, int bits_count, stringstream &ss,
                             bool separate, int sz = -1) {
    for (int bit_pos = 0; bit_pos < bits_count; bit_pos++) {
      if (sz > -1 && bit_pos >= sz)
        break;
      unsigned int current_mask = (1u << (bits_count - bit_pos - 1));
      unsigned int masked = input & current_mask;
      unsigned int shifted = masked >> (bits_count - bit_pos - 1);
      if (separate && bit_pos % 4 == 0 && bit_pos > 0) {
        ss << " ";
      }
      ss << shifted;
    }
  }

  bool same_as(const string &other) {
    string my_string_rep = getStringRep();
    string converted = my_string_rep.substr(0, get_nodes_count(b) * 4);
    return converted == other;
  }

  static string getStringRepBlock(struct block *a_block, bool separate) {
    stringstream ss;

    uint32_t *container = a_block->container;
    uint32_t uint_bits = sizeof(uint32_t) * 8;
    uint32_t nodes_count = get_nodes_count(a_block);
    uint32_t used_bits = nodes_count * 4;
    uint32_t blocks_count = used_bits / uint_bits;
    uint32_t extra_bits = used_bits % uint_bits;
    for (uint32_t blockIndex = 0; blockIndex < blocks_count; blockIndex++) {
      int currentBlock = container[blockIndex];
      pass_to_ss_bin(currentBlock, uint_bits, ss, separate);
      // ss << " - ";
    }

    if (extra_bits > 0) {
      uint32_t remainingBits;
      bits_read(a_block, used_bits - extra_bits, used_bits - 1, &remainingBits);
      remainingBits <<= (uint_bits - extra_bits);
      pass_to_ss_bin(remainingBits, uint_bits, ss, separate, extra_bits);
    }

    return ss.str();
  }

  string getStringRep(bool separate = false) {
    return getStringRepBlock(b, separate);
  }

  string frontierStr() {
    stringstream ss;
    for (int i = 0; i < b->children; i++) {
      uint32_t fval = b->preorders[i];
      ss << fval << ", ";
    }
    return ss.str();
  }
  void printSubBlocks() {
    for (int i = 0; i < b->children; i++) {
      struct block *sb = b->children_blocks[i];
      cout << getStringRepBlock(sb, true) << endl;
    }
  }

  void bitset(uint32_t position) {
    int err_check;
    int was_set_already;
    if ((err_check = bit_set(b, position, &was_set_already))) {
      throw runtime_error("bitset: BITSET ERROR, CODE = " +
                          to_string(err_check));
    }
  }

  bool bitread(uint32_t position) {
    int result;
    int err_check;
    if ((err_check = bit_read(b, position, &result))) {
      throw runtime_error("bitread: BITREAD ERROR, CODE = " +
                          to_string(err_check));
    }

    return result;
  }

  uint32_t bitsread(uint32_t from, uint32_t to) {
    uint32_t result;
    int err_check;
    if ((err_check = bits_read(b, from, to, &result))) {
      throw runtime_error("bitsread: BITSREAD ERROR, CODE = " +
                          to_string(err_check));
    }
    return result;
  }

  void bitswrite(uint32_t from, uint32_t to, uint32_t data) {
    int err_check;
    if ((err_check = bits_write(b, from, to, data))) {
      throw runtime_error("bitswrite: BITSWRITE ERROR, CODE = " +
                          to_string(err_check));
    }
  }

  void bitclear(uint32_t position) {
    int err_check;
    if ((err_check = bit_clear(b, position))) {
      throw runtime_error("bitclear: BITCLEAR ERROR, CODE = " +
                          to_string(err_check));
    }
  }

  uint32_t read_block(uint32_t index) { return b->container[index]; }

  uint32_t bv_size() { return b->container_size * 8 * sizeof(uint32_t); }

  void init_topology_debug(uint32_t size) { enlarge_block_size_to(b, size); }
};

#endif // K2TREEELEMENTSTEST_BLOCK_WRAPPER_HPP

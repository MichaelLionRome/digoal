//
// Created by Cristobal Miranda, 2020
//

#ifndef K2TREEELEMENTSTEST_BLOCK_WRAPPER_HPP
#define K2TREEELEMENTSTEST_BLOCK_WRAPPER_HPP

extern "C" {
#include "block/block.h"
#include "block-topology/block_topology.h"
#include "block-frontier/block_frontier.h"
#include "block/queries_state.h"
}

#include <stdexcept>
#include <string>
#include <sstream>


using namespace std;

class BlockWrapper{
  struct block *b;
  struct queries_state qs;

public:
  BlockWrapper(uint32_t tree_depth){
    b = create_block(tree_depth);
    init_queries_state(&qs, tree_depth);
  }

  ~BlockWrapper(){
    free_block(b);
    finish_queries_state(&qs);
  }

  void insert(unsigned long col, unsigned long row){
    insert_point(b, col, row, &qs);
  }

  bool has(unsigned long col, unsigned long row){
    int result;
    has_point(b, col, row, &qs, &result);
    return result == 1;
  }

  static void pass_to_ss_bin(uint32_t input, uint32_t bits_count, stringstream &ss, int sz=-1){
    for (auto bit_pos = 0; bit_pos < bits_count; bit_pos++) {
      if(sz > - 1 && bit_pos >= sz) break;
      unsigned int current_mask = (1u << (bits_count - bit_pos - 1));
      unsigned int masked = input & current_mask;
      unsigned int shifted = masked >> (bits_count - bit_pos - 1);
      ss << shifted;
    }
  }

  bool same_as(const string &other){
    string my_string_rep = getStringRep();
    string converted = my_string_rep.substr(0, b->bt->nodes_count * 4);
    return converted == other;
  }

  string getStringRep(){
    stringstream ss;
    uint32_t *container = b->bt->bv->container;
    uint32_t container_length = b->bt->bv->container_size;
    uint32_t sizeTraversed = 0;
    uint32_t uint_bits = sizeof(uint32_t) * 8;
    uint32_t bv_size = b->bt->bv->size_in_bits;
    for(uint32_t blockIndex = 0; blockIndex < container_length; blockIndex++){
      if(sizeTraversed + uint_bits > bv_size){
        break;
      }
      int currentBlock = container[blockIndex];

      pass_to_ss_bin(currentBlock, uint_bits, ss);

      sizeTraversed += uint_bits;
    }

    if(sizeTraversed < bv_size){
      uint32_t remainingLength = bv_size - sizeTraversed;
      uint32_t remainingBits;
      bits_read(b->bt->bv, sizeTraversed, bv_size - 1, &remainingBits);
      remainingBits <<= (uint_bits - remainingLength);
      pass_to_ss_bin(remainingBits, uint_bits, ss, remainingLength);
    }

    return ss.str().substr(0, b->bt->nodes_count * 4);
  }
};

#endif //K2TREEELEMENTSTEST_BLOCK_WRAPPER_HPP

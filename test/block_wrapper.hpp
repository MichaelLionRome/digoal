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
#include <iostream>


using namespace std;

class BlockWrapper{
  struct block *b;
  struct queries_state qs;

public:
  BlockWrapper(uint32_t tree_depth){
    b = create_block(tree_depth);
    init_queries_state(&qs, tree_depth);
  }
  BlockWrapper(uint32_t tree_depth, uint32_t max_node_count) : BlockWrapper(tree_depth){
    b->max_node_count = max_node_count;
  }

  ~BlockWrapper() noexcept(false){
    int err_check = free_rec_block(b);
    if(err_check) throw runtime_error("free_rec_block: ERROR WHILE FREEING MEMORY, CODE = " + to_string(err_check));
    err_check  = finish_queries_state(&qs);
    if(err_check) throw runtime_error("finish_queries_state: ERROR WHILE FREEING MEMORY, CODE = " + to_string(err_check));
    cout << "data was freed" << endl;
  }

  void insert(unsigned long col, unsigned long row){
    int err_check = insert_point(b, col, row, &qs);
    if(err_check)
      throw runtime_error("CANT INSERT, ERROR CODE= " +  to_string(err_check));
  }

  bool has(unsigned long col, unsigned long row){
    int result;
    has_point(b, col, row, &qs, &result);
    return result == 1;
  }

  static void pass_to_ss_bin(uint32_t input, uint32_t bits_count, stringstream &ss, bool separate, int sz=-1){
    for (auto bit_pos = 0; bit_pos < bits_count; bit_pos++) {
      if(sz > - 1 && bit_pos >= sz) break;
      unsigned int current_mask = (1u << (bits_count - bit_pos - 1));
      unsigned int masked = input & current_mask;
      unsigned int shifted = masked >> (bits_count - bit_pos - 1);
      if(separate && bit_pos % 4 == 0 && bit_pos > 0) {
        ss << " ";
      }
      ss << shifted;
    }
  }

  bool same_as(const string &other){
    string my_string_rep = getStringRep();
    string converted = my_string_rep.substr(0, b->bt->nodes_count * 4);
    return converted == other;
  }

  static string getStringRepBlock(struct block *a_block, bool separate){
    stringstream ss;
    struct block_topology *bt = a_block->bt;
    struct bitvector *bv = bt->bv;
    uint32_t *container = bv->container;
    uint32_t container_length = bv->container_size;
    uint32_t sizeTraversed = 0;
    uint32_t uint_bits = sizeof(uint32_t) * 8;
    uint32_t nodes_count = bt->nodes_count;
    uint32_t used_bits = nodes_count * 4;
    uint32_t blocks_count = used_bits/ uint_bits;
    uint32_t extra_bits = used_bits % uint_bits;
    for(uint32_t blockIndex = 0; blockIndex < blocks_count; blockIndex++){
      int currentBlock = container[blockIndex];
      pass_to_ss_bin(currentBlock, uint_bits, ss, separate);
      //ss << " - ";
    }

    if(extra_bits > 0){
      uint32_t remainingBits;
      bits_read(a_block->bt->bv, used_bits-extra_bits, used_bits - 1, &remainingBits);
      remainingBits <<= (uint_bits - extra_bits);
      pass_to_ss_bin(remainingBits, uint_bits, ss, separate, extra_bits);
    }

    // return ss.str().substr(0, b->bt->nodes_count * 4);
    return ss.str();
  }

  string getStringRep(bool separate = false){
    return getStringRepBlock(b, separate);
  }

  string frontierStr(){
    stringstream ss;
    struct block_frontier *bf = b->bf;
    for(int i = 0; i < bf->frontier.nof_items; i++){
      uint32_t *fval_ptr;
      int err_code = get_element_at(&bf->frontier, i, (char**)&fval_ptr);
      if(err_code)
        throw runtime_error("error reading frontier at i = " + to_string(i));
      uint32_t fval = *fval_ptr;
      ss << fval << ", ";
    }
    return ss.str();
  }
  void printSubBlocks(){
    struct block_frontier *bf = b->bf;
    struct block **sb;
    for(int i = 0; i < bf->blocks.nof_items; i++){
      int err_code = get_element_at(&bf->blocks, i, (char**)&sb);
      if(err_code)
        throw runtime_error("error reading frontier block at i = " + to_string(i));


      cout << getStringRepBlock(*sb, true) << endl;
    }
  }
};

#endif //K2TREEELEMENTSTEST_BLOCK_WRAPPER_HPP

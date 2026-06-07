#include <stdexcept>
#include <string>
extern "C" {
#include "bitvector.h"
}

using namespace std;

class BitvectorWrapper {
  struct bitvector bitvector_;

  void destroy_bitvector() {
    int err = clean_bitvector(&bitvector_);
    if (err)
      throw runtime_error("There was an error during destruction " +
                          to_string(err));
  }

public:
  BitvectorWrapper(uint32_t nodes_count) {
    int err = init_bitvector(&bitvector_, nodes_count);
    if (err)
      throw runtime_error("There was an error during initialization " +
                          to_string(err));
  }

  ~BitvectorWrapper() { destroy_bitvector(); }

  uint32_t size() { return bitvector_.nodes_count; }
  uint32_t container_size() { return bitvector_.container_size; }

  void bitset(uint32_t position) {
    int err = bit_set(&bitvector_, position);
    if (err)
      throw runtime_error("There was an error during bitset " + to_string(err));
  }

  bool bitread(uint32_t position) {
    int result;
    int err = bit_read(&bitvector_, position, &result);
    if (err)
      throw runtime_error("there was an error during bitread " +
                          to_string(err));

    return result;
  }

  void bitclear(uint32_t position) {
    int err = bit_clear(&bitvector_, position);
    if (err)
      throw runtime_error("there was an error during bitclear " +
                          to_string(err));
  }

  void bitswrite(uint32_t from, uint32_t to, uint32_t to_write) {
    int err = bits_write(&bitvector_, from, to, to_write);
    if (err)
      throw runtime_error("there was an error during bitswrite " +
                          to_string(err));
  }

  uint32_t bitsread(uint32_t from, uint32_t to) {
    uint32_t result;
    int err = bits_read(&bitvector_, from, to, &result);
    if (err)
      throw runtime_error("there was an error during bitsread " +
                          to_string(err));

    return result;
  }

  uint32_t read_block(uint32_t block_index) {
    if (block_index > bitvector_.container_size) {
      throw runtime_error("block index out of range");
    }
    return bitvector_.container[block_index];
  }

  uint32_t *get_container() { return bitvector_.container; }
};

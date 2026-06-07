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
#include <string.h>

#include "block_topology.h"
#include "definitions.h"
#include "memalloc.h"

#include "custom_bv_handling.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

#define CEIL_OF_DIV(dividend, divisor)                                         \
  (((dividend) / (divisor)) + (((dividend) % (divisor)) == 0 ? 0 : 1))
#define BITS_SIZE(type) (sizeof(type) * 8)

uint32_t uint_bits = BITS_SIZE(uint32_t);

/* PRIVATE PROTOTYPES */

int resize_bv_to(struct bitvector *bv, uint32_t new_size);
int shift_bv_right_from(struct bitvector *bv, uint32_t from_location,
                        uint32_t to_location, uint32_t shift_amount);
int shift_left_from(struct bitvector *bv, uint32_t from, uint32_t shift_amount);

int collapse_bits(struct block_topology *bt, uint32_t from, uint32_t to);

/* END PRIVATE PROTOTYPES */

int init_block_topology(struct block_topology *bt, NODES_COUNT_T nodes_count) {
  CHECK_ERR(custom_init_bitvector(&bt->bv, nodes_count));
  set_nodes_count(bt, nodes_count);
  return 0;
}

int child_exists(struct block_topology *bt, NODES_COUNT_T input_node_idx,
                 uint32_t requested_child_position, int *result) {
  struct bitvector *bv = &bt->bv;
  if (input_node_idx >= get_nodes_capacity(bt)) {
    *result = FALSE;
    return SUCCESS_ECODE_K2T;
  }
  int bit_on;
  _SAFE_OP_K2(
      bit_read(bv, input_node_idx * 4 + requested_child_position, &bit_on));
  *result = bit_on;

  return SUCCESS_ECODE_K2T;
}

int read_node(struct block_topology *bt, NODES_COUNT_T node_idx,
              uint32_t *result) {
  CHECK_ERR(bits_read(&bt->bv, 4 * node_idx, 4 * (node_idx + 1) - 1,
                      (uint32_t *)result));
  return SUCCESS_ECODE_K2T;
}

int count_children(struct block_topology *bt, NODES_COUNT_T node_idx,
                   uint32_t *result) {
  if (node_idx >= get_nodes_count(bt)) {
    return 0;
  }

  uint32_t node;
  CHECK_ERR(read_node(bt, node_idx, &node));

  *result = POP_COUNT(node);

  return SUCCESS_ECODE_K2T;
}

int resize_bv_to(struct bitvector *bv, uint32_t new_size) {
  uint32_t new_container_size = CEIL_OF_DIV(new_size, uint_bits);

  if (new_container_size == bv->container_size) {
    return SUCCESS_ECODE_K2T;
  }

  uint32_t *new_container = k2tree_alloc_u32array(new_container_size);

  if (new_container_size > bv->container_size) {
    memcpy(new_container, bv->container, bv->container_size * sizeof(uint32_t));
  } else {
    memcpy(new_container, bv->container, new_container_size * sizeof(uint32_t));
  }

  if (bv->container_size > 0)
    _SAFE_OP_K2(custom_clean_bitvector(bv));

  bv->container = new_container;
  bv->container_size = new_container_size;

  return SUCCESS_ECODE_K2T;
}

int shift_bv_right_from(struct bitvector *bv, uint32_t from_location,
                        uint32_t to_location, uint32_t shift_amount) {
  if (shift_amount == 0) {
    return SUCCESS_ECODE_K2T;
  }

  uint32_t original_size = bv->container_size * uint_bits;

  if (to_location + shift_amount > original_size) {
    CHECK_ERR(resize_bv_to(bv, (uint32_t)to_location + shift_amount));
  }

  int from = MAX((int)to_location - (int)uint_bits, (int)from_location);
  int to = (uint32_t)MIN((int)from + (int)uint_bits - 1, (int)to_location - 1);

  int bits_to_shift = MAX(((int)to_location - 1) - (int)from_location + 1, 0);
  int blocks_to_shift_fully = bits_to_shift / (int)uint_bits;
  int extra_bits_to_shift = bits_to_shift % uint_bits;

  int prev_from = from;

  for (int i = 0; i < blocks_to_shift_fully; i++) {
    uint32_t block_to_shift;
    _SAFE_OP_K2(bits_read(bv, from, to, &block_to_shift));
    _SAFE_OP_K2(bits_write(bv, (uint32_t)from + shift_amount,
                           (uint32_t)to + shift_amount, block_to_shift));

    prev_from = from;

    from -= uint_bits;
    to = prev_from - 1;
  }

  // restore from
  from = prev_from;

  if (extra_bits_to_shift > 0) {
    if (blocks_to_shift_fully > 0) {
      to = from - 1;
      from = to - extra_bits_to_shift + 1;
    }
    if (from <= to) {
      uint32_t to_shift;
      _SAFE_OP_K2(bits_read(bv, from, to, &to_shift));
      _SAFE_OP_K2(
          bits_write(bv, from + shift_amount, to + shift_amount, to_shift));
    }
  }

  // clean up
  int remaining = (int)shift_amount;
  for (uint32_t clean_from = from_location;
       clean_from < from_location + shift_amount; clean_from += uint_bits) {
    int cleaning_bits = MIN(remaining, (int)uint_bits);
    _SAFE_OP_K2(bits_write(bv, clean_from,
                           clean_from + (uint32_t)cleaning_bits - 1, 0));
    remaining -= cleaning_bits;
  }

  return SUCCESS_ECODE_K2T;
}

int enlarge_block_size_to(struct block_topology *bt, uint32_t new_block_size) {
  CHECK_ERR(resize_bv_to(&bt->bv, new_block_size * 4));
  return SUCCESS_ECODE_K2T;
}

int shift_right_nodes_after(struct block_topology *bt, NODES_COUNT_T node_index,
                            NODES_COUNT_T nodes_to_insert) {
  uint32_t nodes_count = get_nodes_count(bt);
  NODES_COUNT_T next_size = nodes_count + nodes_to_insert;
  if (next_size > get_allocated_nodes(bt)) {
    CHECK_ERR(enlarge_block_size_to(bt, next_size));
  }

  CHECK_ERR(shift_bv_right_from(&bt->bv, 4 * (node_index + 1), 4 * nodes_count,
                                4 * nodes_to_insert));

  if ((uint32_t)(node_index + 1) < nodes_count) {
    set_nodes_count(bt, nodes_count + nodes_to_insert);
  }

  return SUCCESS_ECODE_K2T;
}

NODES_COUNT_T get_allocated_nodes(struct block_topology *bt) {
  uint32_t allocated_bits = bt->bv.container_size * sizeof(uint32_t) * 8;
  return (NODES_COUNT_T)(allocated_bits / 4);
}

int mark_child_in_node(struct block_topology *bt, NODES_COUNT_T node_index,
                       uint32_t leaf_child) {
  _SAFE_OP_K2(bit_set(&bt->bv, 4 * node_index + leaf_child));
  return SUCCESS_ECODE_K2T;
}

const uint32_t to_4_bits_table[] = {1 << 3, 1 << 2, 1 << 1, 1 << 0};

int insert_node_at(struct block_topology *bt, NODES_COUNT_T node_index,
                   uint32_t code) {
  // Enlarge if needed
  uint32_t start_position = 4 * node_index;
  uint32_t end_position = start_position + 3; // inclusive
  uint32_t four_bits_rep = to_4_bits_table[code];

  _SAFE_OP_K2(bits_write(&bt->bv, start_position, end_position, four_bits_rep));

  uint32_t nodes_count = get_nodes_count(bt);
  if ((uint32_t)(node_index + 1) > nodes_count) {
    set_nodes_count(bt, node_index + 1);
  }

  return SUCCESS_ECODE_K2T;
}

int extract_sub_bitvector(struct block_topology *bt, uint32_t from, uint32_t to,
                          struct bitvector *result) {
  if (from > to) {
    return EXTRACT_SUB_BITVECTOR_FROM_LESS_THAN_TO;
  }
  uint32_t new_size = to - from + 1;
  uint32_t blocks_to_copy = new_size / uint_bits;
  uint32_t extra_bits = new_size % uint_bits;

  for (uint32_t block_index = 0; block_index < blocks_to_copy; block_index++) {
    uint32_t block;
    uint32_t start = from + block_index * uint_bits;
    uint32_t end = from + (block_index + 1) * uint_bits - 1;
    _SAFE_OP_K2(bits_read(&bt->bv, start, end, &block));
    result->container[block_index] = block;
  }

  if (extra_bits > 0) {
    uint32_t last_bits_start_pos = from + blocks_to_copy * uint_bits;
    uint32_t last_bints_end_pos = last_bits_start_pos + extra_bits - 1;
    uint32_t last_bits;
    _SAFE_OP_K2(bits_read(&bt->bv, last_bits_start_pos, last_bints_end_pos,
                          &last_bits));
    last_bits = last_bits << (uint_bits - extra_bits);
    result->container[blocks_to_copy] = last_bits;
  }

  return SUCCESS_ECODE_K2T;
}

int shift_left_from(struct bitvector *bv, uint32_t from,
                    uint32_t shift_amount) {
  if (shift_amount == 0)
    return SUCCESS_ECODE_K2T;
  uint32_t to = bv->container_size * uint_bits - 1;
  int amount_to_shift_int = (int)to - (int)from + 1;
  if (amount_to_shift_int == 0) {
    return SUCCESS_ECODE_K2T;
  } else if (amount_to_shift_int < 0) {
    return SHIFT_LEFT_FROM_OUT_OF_RANGE_FROM;
  }
  uint32_t amount_to_shift = (uint32_t)amount_to_shift_int;
  uint32_t blocks_to_shift = amount_to_shift / uint_bits;
  uint32_t extra_bits_to_shift = amount_to_shift % uint_bits;

  for (uint32_t i = 0; i < blocks_to_shift; i++) {
    uint32_t read_start_pos = from + i * uint_bits;
    uint32_t read_end_pos = read_start_pos + uint_bits - 1;
    uint32_t block_to_write;
    _SAFE_OP_K2(bits_read(bv, read_start_pos, read_end_pos, &block_to_write));

    uint32_t write_start_pos = from - shift_amount + i * uint_bits;
    uint32_t write_end_pos = write_start_pos + uint_bits - 1;
    _SAFE_OP_K2(bits_write(bv, write_start_pos, write_end_pos, block_to_write));
    /* clean read block */
    if (write_end_pos < read_end_pos) {
      uint32_t bits_to_clean = read_end_pos - (write_end_pos + 1) + 1;
      uint32_t blocks_to_clean = bits_to_clean / uint_bits;
      for (uint32_t j = 0; j < blocks_to_clean; j++) {
        uint32_t from_clean = write_end_pos + 1 + j * uint_bits;
        uint32_t to_clean = write_end_pos + 1 + (j + 1) * uint_bits - 1;
        bits_write(bv, from_clean, to_clean, 0);
      }
      /* clean next block part */
      bits_write(bv, write_end_pos + 1 + blocks_to_clean * uint_bits,
                 read_end_pos, 0);
    }
  }

  if (extra_bits_to_shift > 0) {
    uint32_t read_start_pos = to - extra_bits_to_shift + 1;
    uint32_t read_end_pos = to;
    uint32_t last_part;
    _SAFE_OP_K2(bits_read(bv, read_start_pos, read_end_pos, &last_part));

    uint32_t write_start_pos =
        from - shift_amount + blocks_to_shift * uint_bits;
    uint32_t write_end_pos = write_start_pos + extra_bits_to_shift - 1;
    _SAFE_OP_K2(bits_write(bv, write_start_pos, write_end_pos, last_part));
    /* clean read block */
    if (write_end_pos < read_end_pos) {
      uint32_t bits_to_clean = read_end_pos - (write_end_pos + 1) + 1;
      uint32_t blocks_to_clean = bits_to_clean / uint_bits;
      for (uint32_t j = 0; j < blocks_to_clean; j++) {
        uint32_t from_clean = write_end_pos + 1 + j * uint_bits;
        uint32_t to_clean = write_end_pos + 1 + (j + 1) * uint_bits - 1;
        bits_write(bv, from_clean, to_clean, 0);
      }
      /* clean next block part */
      bits_write(bv, write_end_pos + 1 + blocks_to_clean * uint_bits,
                 read_end_pos, 0);
    }
  }

  return SUCCESS_ECODE_K2T;
}

int collapse_bits(struct block_topology *bt, uint32_t from, uint32_t to) {
  if (from > to)
    return COLLAPSE_BITS_FROM_GREATER_THAN_TO;
  struct bitvector *bv = &bt->bv;
  uint32_t bits_to_collapse = to - from + 1;
  uint32_t blocks_to_collpase = bits_to_collapse / uint_bits;
  uint32_t extra_bits = bits_to_collapse % uint_bits;

  if (bits_to_collapse >= bv->container_size * uint_bits)
    return COLLAPSE_BITS_BITS_DIFF_GTE_THAN_BVSIZE;

  for (uint32_t i = 0; i < blocks_to_collpase; i++) {
    uint32_t from_part = from + i * uint_bits;
    uint32_t to_part = from + (i + 1) * uint_bits - 1;
    _SAFE_OP_K2(bits_write(bv, from_part, to_part, 0));
  }
  if (extra_bits > 0)
    _SAFE_OP_K2(bits_write(bv, to - extra_bits + 1, to, 0));

  CHECK_ERR(shift_left_from(bv, to + 1, bits_to_collapse));
  CHECK_ERR(
      resize_bv_to(&bt->bv, bv->container_size * uint_bits - bits_to_collapse));

  return SUCCESS_ECODE_K2T;
}

int collapse_nodes(struct block_topology *bt, uint32_t from, uint32_t to) {
  CHECK_ERR(collapse_bits(bt, 4 * from, 4 * (to + 1) - 1));
  uint32_t nodes_count = get_nodes_count(bt);
  set_nodes_count(bt, nodes_count - (to - from + 1));
  return SUCCESS_ECODE_K2T;
}

int create_block_topology(struct block_topology *bt) {
  init_block_topology(bt, 0);
  return SUCCESS_ECODE_K2T;
}

int free_block_topology(struct block_topology *bt) {
  if (bt->bv.container_size > 0)
    _SAFE_OP_K2(custom_clean_bitvector(&bt->bv));
  return SUCCESS_ECODE_K2T;
}

uint32_t get_nodes_count(struct block_topology *bt) {
  return bt->bv.nodes_count;
}

int set_nodes_count(struct block_topology *bt, uint32_t nodes_count) {
  if (4 * nodes_count > bt->bv.container_size * uint_bits) {
    printf("Error container not big enough %d > %d\n", 4 * nodes_count,
           bt->bv.container_size * uint_bits);
    exit(1);
  }
  bt->bv.nodes_count = nodes_count;
  return SUCCESS_ECODE_K2T;
}

uint32_t get_nodes_capacity(struct block_topology *bt) {
  return bt->bv.container_size * (uint_bits / 4);
}


#include <bitvector.h>
#include <block.h>
#include <stdlib.h>
#include <string.h>

#define SAFE_OP(op)                                                            \
  do {                                                                         \
    if ((op) != SUCCESS_ECODE) {                                               \
      printf("There was an error while running %s. Error code: %d\n", (#op),   \
             (op));                                                            \
      return (op);                                                             \
    }                                                                          \
  } while (0)

#define BITS_IN_TYPE(input_type) (sizeof(input_type) * 8)
#define BVCTYPE_BITS BITS_IN_TYPE(uint32_t)

#define CONVERT_BITS_TO_CONTAINER_NUM(bits_num, container_type)                \
  (((bits_num) / BITS_IN_TYPE(container_type)) +                               \
   (((bits_num) % BITS_IN_TYPE(container_type)) > 0 ? 1 : 0))

#define CHECK_BOUNDARIES(input_bitvector, position)                            \
  do {                                                                         \
    if ((position) >= (input_bitvector->container_size * BVCTYPE_BITS)) {      \
      fprintf(stderr, "CHECK BOUNDARIES; %d >= %d", (int)position,             \
              (int)(input_bitvector->container_size * BVCTYPE_BITS));          \
      return ERR_OUT_OF_BOUNDARIES;                                            \
    }                                                                          \
  } while (0)

#define BLOCK_INDEX(block_size_in_bits, bit_position)                          \
  ((bit_position) / (block_size_in_bits))

#define POSITION_IN_BLOCK(block_size_in_bits, bit_position)                    \
  ((bit_position) % (block_size_in_bits))

#define BITMASK_FROM_POS_IN_BLOCK(pos_in_block) (1 << (31 - (pos_in_block)))

static inline uint32_t _extract_right_side(uint32_t input_block,
                                           uint32_t extract_index);

int init_bitvector(struct block *input_bitvector, NODES_BV_T nodes_count_) {
  if (!input_bitvector)
    return ERR_NULL_BITVECTOR;

  input_bitvector->container_size =
      CONVERT_BITS_TO_CONTAINER_NUM(nodes_count_ * 4, BVCTYPE);
  input_bitvector->container =
      (BVCTYPE *)calloc(input_bitvector->container_size, sizeof(BVCTYPE));

  return SUCCESS_ECODE;
}

int clean_bitvector(struct block *input_bitvector) {
  if (!input_bitvector)
    return ERR_NULL_BITVECTOR;
  if (!input_bitvector->container)
    return ERR_NULL_BITVECTOR_CONTAINER;

  free(input_bitvector->container);
  input_bitvector->container = NULL;

  return SUCCESS_ECODE;
}

int bit_read(struct block *input_bitvector, uint32_t position, int *result) {
  CHECK_BOUNDARIES(input_bitvector, position);

  BVCTYPE *container = input_bitvector->container;

  uint32_t block_index = BLOCK_INDEX(BVCTYPE_BITS, position);
  uint32_t position_in_block = POSITION_IN_BLOCK(BVCTYPE_BITS, position);
  uint32_t pos_bitmask = BITMASK_FROM_POS_IN_BLOCK(position_in_block);

  *result = (int)((pos_bitmask & container[block_index]) > 0 ? 1 : 0);

  return SUCCESS_ECODE;
}

int bit_set(struct block *input_bitvector, uint32_t position) {
  CHECK_BOUNDARIES(input_bitvector, position);
  int bit_is_set;
  SAFE_OP(bit_read(input_bitvector, position, &bit_is_set));
  if (bit_is_set)
    return SUCCESS_ECODE;

  BVCTYPE *container = input_bitvector->container;

  uint32_t block_index = BLOCK_INDEX(BVCTYPE_BITS, position);
  uint32_t position_in_block = POSITION_IN_BLOCK(BVCTYPE_BITS, position);
  uint32_t pos_bitmask = BITMASK_FROM_POS_IN_BLOCK(position_in_block);

  container[block_index] |= pos_bitmask;

  return SUCCESS_ECODE;
}

int bit_clear(struct block *input_bitvector, uint32_t position) {
  CHECK_BOUNDARIES(input_bitvector, position);
  int bit_is_set;
  SAFE_OP(bit_read(input_bitvector, position, &bit_is_set));
  if (!bit_is_set)
    return SUCCESS_ECODE;

  BVCTYPE *container = input_bitvector->container;

  uint32_t block_index = BLOCK_INDEX(BVCTYPE_BITS, position);
  uint32_t position_in_block = POSITION_IN_BLOCK(BVCTYPE_BITS, position);
  uint32_t pos_bitmask = BITMASK_FROM_POS_IN_BLOCK(position_in_block);

  container[block_index] ^= pos_bitmask;

  return SUCCESS_ECODE;
}

uint32_t right_side_bitmasks[] = {
    (1u << 0u) - 1u, (1u << 1) - 1, (1u << 2) - 1,    // 0, 1, 3 -- ..000,
                                                      // ..001, ..011
    (1u << 3u) - 1u, (1u << 4) - 1, (1u << 5) - 1,    // 7, 15, 31 -- ..111,
                                                      // ..1111, ..11111
    (1u << 6u) - 1u, (1u << 7) - 1, (1u << 8) - 1,    // 63, 127, 255
    (1u << 9u) - 1u, (1u << 10) - 1, (1u << 11) - 1,  // 511, 1023, 2047
    (1u << 12u) - 1u, (1u << 13) - 1, (1u << 14) - 1, // 4095, 8191, ...
    (1u << 15u) - 1u, (1u << 16) - 1, (1u << 17) - 1, (1u << 18u) - 1u,
    (1u << 19) - 1, (1u << 20) - 1, (1u << 21u) - 1u, (1u << 22) - 1,
    (1u << 23) - 1, (1u << 24u) - 1u, (1u << 25) - 1, (1u << 26) - 1,
    (1u << 27u) - 1u, (1u << 28) - 1, (1u << 29) - 1, (1u << 30u) - 1u,
    (1u << 31) - 1, (uint32_t)-1};

static inline uint32_t _extract_right_side(uint32_t input_block,
                                           uint32_t extract_index) {
  return input_block & right_side_bitmasks[extract_index];
}

int bits_write(struct block *input_bitvector, uint32_t from, uint32_t to,
               BVCTYPE to_write) {
  CHECK_BOUNDARIES(input_bitvector, from);
  CHECK_BOUNDARIES(input_bitvector, to);
  if (from > to) {
    return ERR_BITS_WRITE_FROM_GT_TO;
  }

  BVCTYPE *container = input_bitvector->container;

  uint32_t right_pos_in_block = POSITION_IN_BLOCK(BVCTYPE_BITS, to);

  uint32_t left_block_idx = BLOCK_INDEX(BVCTYPE_BITS, from);
  uint32_t right_block_idx = BLOCK_INDEX(BVCTYPE_BITS, to);

  uint32_t bits_to_write = to - from + 1;

  BVCTYPE left_block = container[left_block_idx];
  uint32_t right_bits_to_right = BVCTYPE_BITS - right_pos_in_block - 1;

  if (left_block_idx == right_block_idx) {
    uint32_t parts_to_remove_rshift = left_block >> right_bits_to_right;
    uint32_t part_to_remove =
        _extract_right_side(parts_to_remove_rshift, bits_to_write)
        << right_bits_to_right;
    uint32_t part_to_add = to_write << right_bits_to_right;

    container[left_block_idx] += part_to_add - part_to_remove;
  } else {
    BVCTYPE right_block = container[right_block_idx];
    uint32_t right_block_keep_part =
        _extract_right_side(right_block, right_bits_to_right);
    uint32_t right_extract_to_add =
        _extract_right_side(to_write, right_pos_in_block + 1);
    uint32_t right_part_to_add = right_extract_to_add << right_bits_to_right;
    container[right_block_idx] = right_block_keep_part + right_part_to_add;

    uint32_t to_left_pos_in_block = POSITION_IN_BLOCK(BVCTYPE_BITS, from) - 1;
    uint32_t left_bits_to_right = BVCTYPE_BITS - to_left_pos_in_block - 1;
    uint32_t left_block_part_to_remove =
        _extract_right_side(left_block, left_bits_to_right);
    uint32_t left_block_part_to_add = to_write >> (right_pos_in_block + 1);
    container[left_block_idx] =
        left_block - left_block_part_to_remove + left_block_part_to_add;
  }

  return SUCCESS_ECODE;
}

int bits_read(struct block *input_bitvector, uint32_t from, uint32_t to,
              uint32_t *result) {
  CHECK_BOUNDARIES(input_bitvector, from);
  CHECK_BOUNDARIES(input_bitvector, to);
  if (from > to) {
    return ERR_BITS_WRITE_FROM_GT_TO;
  }

  BVCTYPE *container = input_bitvector->container;

  uint32_t right_pos_in_block = POSITION_IN_BLOCK(BVCTYPE_BITS, to);

  uint32_t left_block_idx = BLOCK_INDEX(BVCTYPE_BITS, from);
  uint32_t right_block_idx = BLOCK_INDEX(BVCTYPE_BITS, to);

  if (left_block_idx == right_block_idx) {
    uint32_t block = container[left_block_idx];
    uint32_t block_shifted = block >> (BVCTYPE_BITS - right_pos_in_block - 1);
    uint32_t bits_to_read = to - from + 1;
    if (bits_to_read == BVCTYPE_BITS) {
      *result = block_shifted;
      return SUCCESS_ECODE;
    }

    *result = _extract_right_side(block_shifted, bits_to_read);
    return SUCCESS_ECODE;
  }

  uint32_t from_pos_in_left_block = POSITION_IN_BLOCK(BVCTYPE_BITS, from) - 1;
  uint32_t right_block = container[right_block_idx];
  uint32_t left_block = container[left_block_idx];

  uint32_t to_shift_right = BVCTYPE_BITS - (right_pos_in_block + 1);
  uint32_t right_block_shifted = right_block >> to_shift_right;

  uint32_t left_shift = right_pos_in_block + 1;
  uint32_t left_block_shifted =
      _extract_right_side(left_block, BVCTYPE_BITS - from_pos_in_left_block - 1)
      << left_shift;

  *result = right_block_shifted + left_block_shifted;
  return SUCCESS_ECODE;
}

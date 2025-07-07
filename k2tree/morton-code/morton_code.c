#include "morton_code.h"

#include "definitions.h"

#include "custom_bv_handling.h"

int init_morton_code(struct morton_code *mc, uint32_t treedepth) {
  mc->treedepth = treedepth;
  CHECK_ERR(custom_init_bitvector(&mc->container, 2 * treedepth));
  return SUCCESS_ECODE;
}

int clean_morton_code(struct morton_code *mc) {
  custom_clean_bitvector(&mc->container);
  return SUCCESS_ECODE;
}

int add_element_morton_code(struct morton_code *mc, uint32_t position,
                            uint32_t code) {
  _SAFE_OP_K2(bits_write(&mc->container, 2 * position, 2 * position + 1, code));
  return SUCCESS_ECODE;
}

int get_code_at_morton_code(struct morton_code *mc, uint32_t position,
                            uint32_t *result) {
  _SAFE_OP_K2(
      bits_read(&mc->container, 2 * position, 2 * position + 1, result));
  return SUCCESS_ECODE;
}

int leaf_child_morton_code(struct morton_code *mc, uint32_t *result) {
  return get_code_at_morton_code(mc, mc->treedepth - 1, result);
}

int convert_coordinates_to_morton_code(ulong col, ulong row, uint32_t treedepth,
                                       struct morton_code *result) {
  ulong current_level = (1L << (ulong)treedepth);

  ulong half_level;
  uint32_t mc_position = 0;
  while ((half_level = current_level >> 1) > 0) {
    uint32_t quadrant;

    if (col >= half_level && row >= half_level) {
      quadrant = 3;
    } else if (col >= half_level && row < half_level) {
      quadrant = 2;
    } else if (col < half_level && row >= half_level) {
      quadrant = 1;
    } else {
      quadrant = 0;
    }

    col %= half_level;
    row %= half_level;
    current_level = half_level;

    _SAFE_OP_K2(add_element_morton_code(result, mc_position++, quadrant));
  }
  return SUCCESS_ECODE;
}

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
#include "morton_code.h"

#include "custom_bv_handling.h"

void init_morton_code(struct morton_code *mc, uint32_t treedepth) {
  mc->treedepth = treedepth;
  mc->container =
      (MC_CONTAINER_T *)malloc(2 * mc->treedepth * sizeof(MC_CONTAINER_T));
}

void clean_morton_code(struct morton_code *mc) {
  if (mc->container) {

    free(mc->container);
    mc->container = NULL;
  }
}

void add_element_morton_code(struct morton_code *mc, uint32_t position,
                             uint32_t code) {
  mc->container[2 * position] = (code & 2) > 0 ? 1 : 0;
  mc->container[2 * position + 1] = (code & 1) > 0 ? 1 : 0;
}

uint32_t get_code_at_morton_code(struct morton_code *mc, uint32_t position) {
  return (mc->container[2 * position] << 1) + mc->container[2 * position + 1];
}

uint32_t leaf_child_morton_code(struct morton_code *mc) {
  return get_code_at_morton_code(mc, mc->treedepth - 1);
}

void convert_coordinates_to_morton_code(ulong col, ulong row,
                                        uint32_t treedepth,
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

    add_element_morton_code(result, mc_position++, quadrant);
  }
}

int convert_morton_code_to_coordinates(struct morton_code *input_mc,
                                       struct pair2dl *result) {
  return convert_morton_code_to_coordinates_select_treedepth(
      input_mc, result, input_mc->treedepth);
}

int convert_morton_code_to_coordinates_select_treedepth(
    struct morton_code *input_mc, struct pair2dl *result,
    TREE_DEPTH_T treedepth) {
  long col = 0;
  long row = 0;
  for (unsigned int i = 0; i < treedepth; i++) {
    uint32_t current = get_code_at_morton_code(input_mc, i);
    uint32_t current_pow = treedepth - 1 - i;
    switch (current) {
    case 3:
      col += 1 << current_pow;
      row += 1 << current_pow;
      break;
    case 2:
      col += 1 << current_pow;
      break;
    case 1:
      row += 1 << current_pow;
      break;
    case 0:
      break;
    default:
      return INVALID_MC_VALUE;
      break;
    }
  }
  result->col = col;
  result->row = row;

  return SUCCESS_ECODE_K2T;
}

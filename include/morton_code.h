#ifndef _MORTON_CODE_H
#define _MORTON_CODE_H

#include <bitvector.h>
#include <stdint.h>

#include "definitions.h"

struct morton_code {
  struct bitvector container;
  uint32_t treedepth;
};

int init_morton_code(struct morton_code *mc, uint32_t treedepth);
int clean_morton_code(struct morton_code *mc);
int add_element_morton_code(struct morton_code *mc, uint32_t position,
                            uint32_t code);
int get_code_at_morton_code(struct morton_code *mc, uint32_t position,
                            uint32_t *result);
int leaf_child_morton_code(struct morton_code *mc, uint32_t *result);
int convert_coordinates_to_morton_code(ulong col, ulong row, uint32_t treedepth,
                                       struct morton_code *result);

int convert_morton_code_to_coordinates(struct morton_code *input_mc,
                                       struct pair2dl *result);
#endif /* _MORTON_CODE_H */

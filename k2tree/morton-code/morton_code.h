#ifndef _MORTON_CODE_H
#define _MORTON_CODE_H

#include <stdint.h>

#include <bitvector.h>

typedef unsigned long ulong;

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

#endif /* _MORTON_CODE_H */

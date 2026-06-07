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
#ifndef _MORTON_CODE_H
#define _MORTON_CODE_H

#include <stdint.h>

#include "definitions.h"

#define MC_CONTAINER_T uint8_t

struct morton_code {
  MC_CONTAINER_T *container;
  uint32_t treedepth;
};

void init_morton_code(struct morton_code *mc, uint32_t treedepth);
void clean_morton_code(struct morton_code *mc);
void add_element_morton_code(struct morton_code *mc, uint32_t position,
                             uint32_t code);
uint32_t get_code_at_morton_code(struct morton_code *mc, uint32_t position);
uint32_t leaf_child_morton_code(struct morton_code *mc);
void convert_coordinates_to_morton_code(ulong col, ulong row,
                                        uint32_t treedepth,
                                        struct morton_code *result);

int convert_morton_code_to_coordinates(struct morton_code *input_mc,
                                       struct pair2dl *result);
#endif /* _MORTON_CODE_H */

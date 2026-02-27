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
#ifndef _K2TREE_DEFINITIONS_H_
#define _K2TREE_DEFINITIONS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector.h>

#ifndef SUCCESS_ECODE
#define SUCCESS_ECODE 0;
#endif

#define NOT_IMPLEMENTED -100
#define DOES_NOT_EXIST_CHILD_ERR 1
#define FRONTIER_OUT_OF_BOUNDS 2
#define FRONTIER_NODE_WITHIN_FIND_INSERTION_LOC 3
#define EXTRACT_SUB_BITVECTOR_FROM_LESS_THAN_TO 4
#define SHIFT_LEFT_FROM_OUT_OF_RANGE_FROM 5
#define COLLAPSE_BITS_FROM_GREATER_THAN_TO 6
#define COLLAPSE_BITS_BITS_DIFF_GTE_THAN_BVSIZE 7
#define FIX_INDEXES_PREORDER_HIGHER_THAN_DELTA 8
#define RESET_SIZE_HIGHER_THAN_CAPACITY 9
#define K2TREE_ERR_NULL_BITVECTOR 10
#define K2TREE_ERR_NULL_BITVECTOR_CONTAINER 11
#define INVALID_MC_VALUE 12

#define CHECK_ERR(err)                                                         \
  do {                                                                         \
    if (err != SUCCESS_ECODE) {                                                \
      return (err);                                                            \
    }                                                                          \
  } while (0)

#define CHECK_CHILD_ERR(err)                                                   \
  do {                                                                         \
    if (err != SUCCESS_ECODE && err != DOES_NOT_EXIST_CHILD_ERR) {             \
      return (err);                                                            \
    }                                                                          \
  } while (0)
/*
#define _SAFE_OP_K2(op)                                                        \
  do {                                                                         \
    if ((op) != SUCCESS_ECODE) {                                               \
      printf("There was an error while running %s. Error code: %d\n", (#op),   \
             (op));                                                            \
      exit(op);                                                                \
    }                                                                          \
  } while (0)
*/

#define _SAFE_OP_K2(op) op

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define DEFINE_READ_ELEMENT_PROTOTYPE(typename, type)                          \
  type read_##typename##_element(struct vector *v, uint32_t position);

#define DEFINE_READ_ELEMENT(typename, type)                                    \
  type read_##typename##_element(struct vector *v, uint32_t position) {        \
    type *data = (type *)v->data;                                              \
    return *(data + position);                                                 \
  }

#ifndef POP_COUNT
#define POP_COUNT(u32_input) __builtin_popcount(u32_input)
#endif

typedef unsigned long ulong;

#ifndef MAX_NODES_IN_BLOCK
#define MAX_NODES_IN_BLOCK 1024
#endif
#ifndef STARTING_BLOCK_CAPACITY
#define STARTING_BLOCK_CAPACITY 64
#endif

DEFINE_READ_ELEMENT_PROTOTYPE(uint, uint32_t)
DEFINE_READ_ELEMENT_PROTOTYPE(block, struct block *)

struct pair2dl {
  long col;
  long row;
};

#endif /* _K2TREE_DEFINITIONS_H_ */

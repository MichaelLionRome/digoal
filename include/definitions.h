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

#ifndef SUCCESS_ECODE_K2T
#define SUCCESS_ECODE_K2T 0
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

// non error
#define LAZY_STOP_ECODE_K2T 100

#define CHECK_ERR(err)                                                         \
  do {                                                                         \
    if (err != SUCCESS_ECODE_K2T) {                                            \
      return (err);                                                            \
    }                                                                          \
  } while (0)

#define CHECK_CHILD_ERR(err)                                                   \
  do {                                                                         \
    if (err != SUCCESS_ECODE_K2T && err != DOES_NOT_EXIST_CHILD_ERR) {         \
      return (err);                                                            \
    }                                                                          \
  } while (0)

#define _SAFE_OP_K2(op)                                                        \
  do {                                                                         \
    if ((op) != SUCCESS_ECODE_K2T) {                                           \
      printf("There was an error while running %s. Error code: %d\n", (#op),   \
             (op));                                                            \
      exit(op);                                                                \
    }                                                                          \
  } while (0)

//#define _SAFE_OP_K2(op) op

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef POP_COUNT
#define POP_COUNT(u32_input) __builtin_popcount(u32_input)
#endif

typedef unsigned long ulong;

#ifndef MAX_NODES_IN_BLOCK
#define MAX_NODES_IN_BLOCK 256
#endif
#ifndef STARTING_BLOCK_CAPACITY
#define STARTING_BLOCK_CAPACITY 64
#endif

typedef struct pair2dl {
  long col;
  long row;
} pair2dl_t;

// typedef struct pair2dl pair2dl_t;

typedef enum { COLUMN_COORD = 0, ROW_COORD = 1 } coord_t;

struct sip_ipoint {
  long coord;
  coord_t coord_type;
};

struct node_subtree_info {
  uint32_t node_index;
  uint32_t node_relative_depth;
  uint32_t subtree_size;
};

typedef struct node_subtree_info nsi_t;

typedef uint8_t TREE_DEPTH_T;
typedef uint16_t MAX_NODE_COUNT_T;
typedef uint32_t BLOCK_INDEX_T;

#endif /* _K2TREE_DEFINITIONS_H_ */

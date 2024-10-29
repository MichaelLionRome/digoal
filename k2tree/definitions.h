#ifndef _K2TREE_DEFINITIONS_H_
#define _K2TREE_DEFINITIONS_H_

#include <stdio.h>
#include <stdlib.h>

#define CHECK_ERR(err)                                                         \
  do {                                                                         \
    if (err) {                                                                 \
      return (err);                                                            \
    }                                                                          \
  } while (0)

#define SAFE_OP(op)                                                            \
  do {                                                                         \
    if ((op) != SUCCESS_ECODE) {                                               \
      printf("There was an error while running %s. Error code: %d\n", (#op),   \
             (op));                                                            \
      exit(op);                                                                \
    }                                                                          \
  } while (0)

#ifndef SUCCESS_ECODE
#define SUCCESS_ECODE 0;
#endif
#define DOES_NOT_EXIST_CHILD_ERR 1;

#define NOT_IMPLEMENTED -100

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define FRONTIER_OUT_OF_BOUNDS 2

#define DEFINE_READ_ELEMENT(typename, type)                                    \
  type read_##typename##_element(struct vector *v, int position) {             \
    type *data = (type *)v->data;                                              \
    return *(data + position);                                                 \
  }

DEFINE_READ_ELEMENT(uint, uint32_t)
DEFINE_READ_ELEMENT(block, struct block *)

#define POP_COUNT(u32_input) __builtin_popcount(u32_input)

#endif

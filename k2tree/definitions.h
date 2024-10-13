#ifndef _K2TREE_DEFINITIONS_H_
#define _K2TREE_DEFINITIONS_H_

#define CHECK_ERR(err)                                                         \
  do {                                                                         \
    if (err) {                                                                 \
      return (err);                                                            \
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
    type out;                                                                  \
    char *vread_result;                                                        \
    get_element_at(v, position, &vread_result);                                \
    memcpy(&out, vread_result, v->element_size);                               \
    return out;                                                                \
  }

DEFINE_READ_ELEMENT(uint, uint32_t)
DEFINE_READ_ELEMENT(block, struct block *)

#endif

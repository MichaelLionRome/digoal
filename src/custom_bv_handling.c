
#include "custom_bv_handling.h"

#include "memalloc.h"

#include "definitions.h"

#define BITS_IN_TYPE(input_type) (sizeof(input_type) * 8)
#define BVCTYPE_BITS BITS_IN_TYPE(uint32_t)

#define CONVERT_BITS_TO_CONTAINER_NUM(bits_num, container_type)                \
  (((bits_num) / BITS_IN_TYPE(container_type)) +                               \
   (((bits_num) % BITS_IN_TYPE(container_type)) > 0 ? 1 : 0))

int custom_init_bitvector(struct bitvector *input_bitvector,
                          uint32_t size_in_bits_) {
  if (!input_bitvector)
    return K2TREE_ERR_NULL_BITVECTOR;

  input_bitvector->container_size =
      CONVERT_BITS_TO_CONTAINER_NUM(size_in_bits_, uint32_t);

  struct u32array_alloc container_alloc =
      k2tree_alloc_u32array(input_bitvector->container_size);
  input_bitvector->container = container_alloc.data;
  input_bitvector->size_in_bits = size_in_bits_;

  return SUCCESS_ECODE;
}

int custom_clean_bitvector(struct bitvector *input_bitvector) {
  if (!input_bitvector)
    return K2TREE_ERR_NULL_BITVECTOR;
  if (!input_bitvector->container)
    return K2TREE_ERR_NULL_BITVECTOR_CONTAINER;

  struct u32array_alloc to_free;
  to_free.data = input_bitvector->container;
  to_free.size = input_bitvector->container_size;
  k2tree_free_u32array(to_free);
  input_bitvector->container = NULL;

  return SUCCESS_ECODE;
}

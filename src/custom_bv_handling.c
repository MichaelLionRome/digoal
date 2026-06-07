
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

#include "custom_bv_handling.h"
#include <assert.h>
#include <block.h>

#include "memalloc.h"

#include "definitions.h"

#define BITS_IN_TYPE(input_type) (sizeof(input_type) * 8)
#define BVCTYPE_BITS BITS_IN_TYPE(uint32_t)

#define CONVERT_BITS_TO_CONTAINER_NUM(bits_num, container_type)                \
  (((bits_num) / BITS_IN_TYPE(container_type)) +                               \
   (((bits_num) % BITS_IN_TYPE(container_type)) > 0 ? 1 : 0))

int custom_init_bitvector(struct block *input_bitvector,
                          NODES_BV_T nodes_count) {
  if (!input_bitvector)
    return K2TREE_ERR_NULL_BITVECTOR;

  input_bitvector->container_size =
      CONVERT_BITS_TO_CONTAINER_NUM(nodes_count * 4, uint32_t);

  input_bitvector->container = NULL;
  if (input_bitvector->container_size > 0) {
    input_bitvector->container =
        k2tree_alloc_u32array(input_bitvector->container_size);
  }

  input_bitvector->nodes_count = nodes_count;

  return SUCCESS_ECODE_K2T;
}

int custom_clean_bitvector(struct block *input_bitvector) {
  if (!input_bitvector)
    return K2TREE_ERR_NULL_BITVECTOR;
  if (!input_bitvector->container)
    return K2TREE_ERR_NULL_BITVECTOR_CONTAINER;

  uint32_t *to_free = input_bitvector->container;
  if (to_free) {
    k2tree_free_u32array(to_free);
    input_bitvector->container = NULL;
  }

  return SUCCESS_ECODE_K2T;
}

#ifndef _BITVECTOR_H_
#define _BITVECTOR_H_

#include <stdint.h>
#include <stdio.h>

#define SUCCESS_ECODE 0
#define ERR_NULL_BITVECTOR -1
#define ERR_NULL_BITVECTOR_CONTAINER -2
#define ERR_OUT_OF_BOUNDARIES -3
#define ERR_BITS_WRITE_FROM_GT_TO -4

// TODO: refactor to allow custom BVCTYPE
typedef uint32_t BVCTYPE;

#ifdef VERY_LIGHT_FIELDS
typedef uint8_t CONTAINER_SZ_T;
typedef uint8_t NODES_BV_T;
#elif defined(LIGHT_FIELDS)
typedef uint16_t CONTAINER_SZ_T;
typedef uint16_t NODES_BV_T;

#else
typedef uint32_t CONTAINER_SZ_T;
typedef uint32_t NODES_BV_T;
#endif

struct block;

int init_bitvector(struct block *input_bitvector, NODES_BV_T nodes_count_);
int clean_bitvector(struct block *input_bitvector);

int bit_read(struct block *input_bitvector, uint32_t position, int *result);
int bit_set(struct block *input_bitvector, uint32_t position,
            int *bit_was_set_already);
int bit_clear(struct block *input_bitvector, uint32_t position);
int bits_write(struct block *input_bitvector, uint32_t from, uint32_t to,
               BVCTYPE to_write);
int bits_read(struct block *input_bitvector, uint32_t from, uint32_t to,
              uint32_t *result);

int bits_write_bv(struct block *input_bitvector, struct block *output_bitvector,
                  int start_src, int start_dst, int length);

int bits_write_uarray_small(uint32_t *input_uarr, int sz, uint32_t from,
                            uint32_t to, uint32_t to_write);
int bits_read_uarray_small(uint32_t *input_uarr, int sz, uint32_t from,
                           uint32_t to, uint32_t *result);

int bits_write_uarray(uint32_t *input_uarr, int input_size,
                      uint32_t *output_uarr, int output_size, int start_src,
                      int start_dst, int length);

#endif /* _BITVECTOR_H_ */

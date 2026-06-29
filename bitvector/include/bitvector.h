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
typedef uint16_t SZ_BITS_T;
typedef uint8_t CONTAINER_SZ_T;
typedef uint8_t NODES_BV_T;
#elif defined(LIGHT_FIELDS)
typedef uint32_t SZ_BITS_T;
typedef uint16_t CONTAINER_SZ_T;
typedef uint16_t NODES_BV_T;

#else
typedef uint32_t SZ_BITS_T;
typedef uint32_t CONTAINER_SZ_T;
typedef uint32_t NODES_BV_T;
#endif

struct block;

int init_bitvector(struct block *input_bitvector, NODES_BV_T nodes_count_);
int clean_bitvector(struct block *input_bitvector);

int bit_read(struct block *input_bitvector, uint32_t position, int *result);
int bit_set(struct block *input_bitvector, uint32_t position);
int bit_clear(struct block *input_bitvector, uint32_t position);
int bits_write(struct block *input_bitvector, uint32_t from, uint32_t to,
               BVCTYPE to_write);
int bits_read(struct block *input_bitvector, uint32_t from, uint32_t to,
              uint32_t *result);

#endif /* _BITVECTOR_H_ */

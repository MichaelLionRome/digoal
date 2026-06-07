#ifndef _VECTORS_H_
#define _VECTORS_H_

#include "definitions.h"

#include <vector.h>

struct block;

typedef struct block *block_ptr_t;

define_cvector(uint16_t, int16_t) define_cvector(block_ptr_t, int16_t)
    define_cvector(pair2dl_t, long)

#endif

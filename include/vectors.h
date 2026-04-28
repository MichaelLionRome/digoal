#ifndef _VECTORS_H_
#define _VECTORS_H_

#include "definitions.h"

#include <vector.h>

struct block;

typedef struct block *block_ptr_t;

define_cvector(uint32_t, int) define_cvector(pair2dl_t, long)
    define_cvector(block_ptr_t, int)

#endif

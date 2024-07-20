#ifndef _BLOCK_TOPOLOGY_H
#define _BLOCK_TOPOLOGY_H

#include <stdint.h>

#include "bitvector.h"

struct block_topology {
    struct bitvector *bv;
    uint32_t nodes_count;
};

#endif /* _BLOCK_TOPOLOGY_H */
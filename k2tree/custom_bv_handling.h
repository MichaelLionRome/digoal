#ifndef _CUSTOM_BV_HANDLING_H
#define _CUSTOM_BV_HANDLING_H

#include <bitvector.h>
#include <stdint.h>

int custom_init_bitvector(struct bitvector *input_bitvector,
                          uint32_t size_in_bits_);
int custom_clean_bitvector(struct bitvector *input_bitvector);

#endif /* _CUSTOM_BV_HANDLING_H */

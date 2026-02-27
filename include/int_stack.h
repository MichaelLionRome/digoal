#ifndef _INT_STACK_H
#define _INT_STACK_H

#include <stdlib.h>

#include "memalloc.h"

struct int_stack {
  struct u32array_alloc cont;
  int capacity;
  int index;
};

void init_int_stack(struct int_stack *s, int capacity);
void free_int_stack(struct int_stack *s);
void reset_int_stack(struct int_stack *s);

void push_int_stack(struct int_stack *s, int value);
int pop_int_stack(struct int_stack *s);
int empty_int_stack(struct int_stack *s);
int top_int_stack(struct int_stack *s);

#endif

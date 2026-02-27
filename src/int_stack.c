#include "int_stack.h"
#include "memalloc.h"
#include <stdio.h>

void init_int_stack(struct int_stack *s, int capacity) {
  s->cont = k2tree_alloc_u32array(capacity);
  s->capacity = capacity;
  s->index = -1;
}

void free_int_stack(struct int_stack *s) { k2tree_free_u32array(s->cont); }

void reset_int_stack(struct int_stack *s) { s->index = -1; }

void push_int_stack(struct int_stack *s, int value) {
  if (s->index + 1 >= s->capacity) {
    fprintf(stderr, "push_int_stack: int_stack at max capacity");
    exit(1);
  }
  s->cont.data[++s->index] = (uint32_t)value;
}

int pop_int_stack(struct int_stack *s) {
  if (s->index == -1) {
    fprintf(stderr, "pop_int_stack: int_stack is empty");
    exit(1);
  }
  int result = (int)s->cont.data[s->index];
  s->index--;
  return result;
}

int empty_int_stack(struct int_stack *s) { return s->index == -1; }

int top_int_stack(struct int_stack *s) { return (int)s->cont.data[s->index]; }

#ifndef _STACK_CREATOR_H
#define _STACK_CREATOR_H

#include <stdio.h>
#include <stdlib.h>

#define define_stack_of_type(type)                                             \
  struct type##_stack {                                                        \
    type *data;                                                                \
    long capacity;                                                             \
    long index;                                                                \
  };                                                                           \
  void init_##type##_stack(struct type##_stack *s, int capacity);              \
  void free_##type##_stack(struct type##_stack *s);                            \
  void reset_##type##_stack(struct type##_stack *s);                           \
  void push_##type##_stack(struct type##_stack *s, type value);                \
  type pop_##type##_stack(struct type##_stack *s);                             \
  int empty_##type##_stack(struct type##_stack *s);                            \
  type top_##type##_stack(struct type##_stack *s);                             \
  type *top_ref_##type##_stack(struct type##_stack *s);

#define declare_stack_of_type(type)                                            \
  void init_##type##_stack(struct type##_stack *s, int capacity) {             \
    s->data = (type *)malloc(capacity * sizeof(type));                         \
    s->capacity = capacity;                                                    \
    s->index = -1;                                                             \
  }                                                                            \
  void free_##type##_stack(struct type##_stack *s) { free(s->data); }          \
  void reset_##type##_stack(struct type##_stack *s) { s->index = -1; }         \
  void push_##type##_stack(struct type##_stack *s, type value) {               \
    /*  if (s->index + 1 >= s->capacity) {                                     \
       fprintf(stderr, "push_"#type"_stack: "#type"_stack at max capacity");   \
       exit(1);                                                                \
     }\ */                                                                     \
    s->data[++s->index] = value;                                               \
  }                                                                            \
  type pop_##type##_stack(struct type##_stack *s) {                            \
    /*   if (s->index == -1) {                                                 \
        fprintf(stderr, "pop_"#type"_stack: "#type"_stack is empty");          \
        exit(1);                                                               \
      }\ */                                                                    \
    type result = s->data[s->index];                                           \
    s->index--;                                                                \
    return result;                                                             \
  }                                                                            \
  int empty_##type##_stack(struct type##_stack *s) { return s->index == -1; }  \
  type top_##type##_stack(struct type##_stack *s) {                            \
    return s->data[s->index];                                                  \
  }                                                                            \
  type *top_ref_##type##_stack(struct type##_stack *s) {                       \
    return s->data + s->index;                                                 \
  }

#endif

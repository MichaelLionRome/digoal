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
  type *top_ref_##type##_stack(struct type##_stack *s);                        \
  int size_##type##_stack(struct type##_stack *s);

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
  }                                                                            \
  int size_##type##_stack(struct type##_stack *s) { return s->index + 1; }

#endif

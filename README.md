# K2treeDyn

# Build

* Build tested on Ubuntu 18.04
* Most likely won't work on Windows, but might work with custom compilation flow, the only platform dependent code
is __builtin_popcount, can be redefined in definitions.h (with POP_COUNT(u32_input))
* Untested on OS X

`
make
`

# Test

* Required CMake 3.14

gtest will be installed with the make test-build command


`
make test-all
`


# Usage

The generated code is a static library. There are 4 alternatives to link to,
the easiest to use is bin/libk2tree_merged.a which is merged with other libraries which
this project is dependent on.

### Differences in static lib variants

* bin/libk2tree_merged_noalloc.a: Merged with dependencies, but allows for custom memory allocation (implementing memalloc.h).
* bin/libk2tree_merged.a: Comes with default memory allocation (free, malloc, calloc)
* bin/libk2tree_noalloc.a: Unmerged dependencies with abstract allocation
* bin/libk2tree.a: Unmerged dependencies with default memory allocation

There are more variants which differ in the use of the -fPIC flag to compile the dependencies

### Linking

When compiling your source code with this lib add -L{PATH_TO_BASE}/bin -l{preferred static lib} -I${PATH_TO_BASE}/include.
Having -l for example: -lk2tree_merged.

### Code usage

```c

#include <block.h>
#include <queries_state.h>

#include <stdio.h>

int main(void){
    uint32_t treedepth = 5;
    ulong side = 1u << treedepth;
    struct block *root_block = create_block(treedepth);

    struct queries_state qs;
    init_queries_state(&qs, treedepth);

    for (ulong col = 0; col < side; col++) {
        for (ulong row = 0; row < side; row++) {
            // printf("inserting col=%lu, row=%lu\n", col, row);
            insert_point(root_block, col, row, &qs);
        }
    }

    int err_code = free_rec_block(root_block);
    if (err_code)
        printf("There was an error with free_rec_block: %d\n", err_code);
    err_code = finish_queries_state(&qs);
    if (err_code)
        printf("There was an error with finish_queries_state: %d\n", err_code);

    if (!err_code) {
        printf("No errors while freeing data\n");
    }
}
```


Also see examples.


# API

(From block.h)

```c
int has_point(struct block *input_block, ulong col, ulong row,
              struct queries_state *qs, int *result);
int insert_point(struct block *input_block, ulong col, ulong row,
                 struct queries_state *qs);

int naive_scan_points(struct block *input_block, struct queries_state *qs,
                      struct vector *result);

int report_column(struct block *input_block, ulong col,
                  struct queries_state *qs, struct vector *result);
int report_row(struct block *input_block, ulong row, struct queries_state *qs,
               struct vector *result);

struct block *create_block(TREE_DEPTH_T tree_depth);
int free_rec_block(struct block *input_block);
int free_block(struct block *input_block);
```

### `has_point`

Receives an `input_block` which must be the root block, also the coordinates
of the matrix to be checked within the range [0, 2^treedepth - 1] both `col` and `row`.


* `queries_state` is a state structure to avoid doing too much memory allocation. The same can be used
for all queries among insert, lookup, reporting and scanning.

* `result` is the address to store the result (boolean) of the query

### `naive_scan_points`

Naive means that this is not the most optimized version of a scan of all the points, but nonetheless works good
enough.

Stores all the points stored in the k2tree into a vector of `struct pair2dl`:

```c
struct pair2dl {
  long col;
  long row;
};
```

a `struct vector` is a self increasing array, see: https://github.com/CristobalM/c-vector/blob/master/include/vector.h).

* `input_block` the root block of the tree
* `queries_state` struct holding state info
* `result` 

### 



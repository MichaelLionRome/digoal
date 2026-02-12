# K2treeDyn

This is an implementation of the k^2 tree described in the following paper:

Arroyuelo, Diego & de Bernardo, Guillermo & Gagie, Travis & Navarro, Gonzalo. (2019). ***Faster Dynamic Compressed d-ary Relations.*** 10.1007/978-3-030-32686-9_30.

# Build

* Build tested on Ubuntu 18.04, 19.10, 20.04; Debian 10.6 (buster)
* Most likely won't work on Windows, but might work with custom compilation flow, the only platform dependent code
is __builtin_popcount, can be redefined in definitions.h (with POP_COUNT(u32_input))
* Untested on OS X

```
./fetch_deps.sh
mkdir build
cd build
cmake ..
make
```

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

A `struct vector` is a self increasing array, see: https://github.com/CristobalM/c-vector/blob/master/include/vector.h).

* `*input_block` the root block of the tree
* `*queries_state` struct holding state info
* `*result` Holds the resulting points, must receive an address to an initialized `struct vector`

### report_column

* `*input_block` the root block of the tree
* `col` column to report
* `*qs` struct holding state info
* `*result` output of points in the column

### report_row

Analogous to `report_column`

### create_block

Initializes a block, for the user only use this with the root block, all other inner blocks will be initialized with
inserts.

* `tree_depth`: Depth of the tree, this is fixed so that coordinates of points can only be in the range `[0, 2^treedepth-1]`. If a bigger range is needed, a new tree must be created. Attempting to insert coordinates into a bigger range will throw an error code in the corresponding calling function.
* TREE_DEPTH_T is of type `uint8_t`, a 8 bits unsigned int which allows depth of at most 255.


### free_rec_block

Will recursively deallocate a block and its children blocks starting at `input_block`


### free_block

Will deallocate a single specified block


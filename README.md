# K2treeDyn

This is an implementation of the k^2 tree described in the following paper:

Arroyuelo, Diego & de Bernardo, Guillermo & Gagie, Travis & Navarro, Gonzalo. (2019). ***Faster Dynamic Compressed d-ary Relations.*** 10.1007/978-3-030-32686-9_30.

# Build

* Build tested on Ubuntu 18.04, 19.10, 20.04; Debian 10.6 (buster)
* Most likely won't work on Windows, but might work with custom compilation flow, the only platform dependent code
is __builtin_popcount, can be redefined in definitions.h (with POP_COUNT(u32_input))
* Working on macOS Monterey

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

int main(void) {
  for (int i = 0; i < 10; i++) {
    uint32_t treedepth = 5;
    unsigned long side = 1u << treedepth;
    struct block *root_block = create_block();

    struct queries_state qs;
    init_queries_state(&qs, treedepth, MAX_NODES_IN_BLOCK, root_block);

    int point_exists;
    for (unsigned long col = 0; col < side; col++) {
      for (unsigned long row = 0; row < side; row++) {
        // printf("inserting col=%lu, row=%lu\n", col, row);
        insert_point(root_block, col, row, &qs, &point_exists);
      }
    }

    printf("Done inserting i=%d\n", i);

    // getchar();

    int err_code = free_rec_block(root_block);
    if (err_code)
      printf("There was an error with free_rec_block: %d\n", err_code);
    err_code = finish_queries_state(&qs);
    if (err_code)
      printf("There was an error with finish_queries_state: %d\n", err_code);

    if (!err_code) {
      printf("No errors while freeing data\n");
    }

    // getchar();
  }

  return 0;
}

```


Also see examples.


# API: Only blocks tree

(From block.h)

```c

int has_point(struct block *input_block, unsigned long col, unsigned long row,
              struct queries_state *qs, int *result);

int insert_point(struct block *input_block, unsigned long col, unsigned long row,
                 struct queries_state *qs, int *already_exists);

int delete_point(struct block *input_block, unsigned long col, unsigned long row,
                 struct queries_state *qs, int *already_not_exists);

int naive_scan_points(struct block *input_block, struct queries_state *qs,
                      struct vector_pair2dl_t *result);

int scan_points_interactively(struct block *input_block,
                              struct queries_state *qs,
                              point_reporter_fun_t point_reporter,
                              void *report_state);

int report_column(struct block *input_block, unsigned long col,
                  struct queries_state *qs, struct vector_pair2dl_t *result);

int report_row(struct block *input_block, unsigned long row, struct queries_state *qs,
               struct vector_pair2dl_t *result);

int report_column_interactively(struct block *input_block, unsigned long col,
                                struct queries_state *qs,
                                point_reporter_fun_t point_reporter,
                                void *report_state);

int report_row_interactively(struct block *input_block, unsigned long row,
                             struct queries_state *qs,
                             point_reporter_fun_t point_reporter,
                             void *report_state);

int sip_join(struct sip_join_input input, coord_reporter_fun_t coord_reporter,
             void *report_state);

struct block *create_block(void);

int free_rec_block(struct block *input_block);

int free_block(struct block *input_block);

struct k2tree_measurement measure_tree_size(struct block *input_block);

int naive_scan_points_lazy_init(struct block *input_block,
                                struct queries_state *qs,
                                struct lazy_handler_naive_scan_t *lazy_handler);

int naive_scan_points_lazy_clean(
    struct lazy_handler_naive_scan_t *lazy_handler);

int naive_scan_points_lazy_next(struct lazy_handler_naive_scan_t *lazy_handler,
                                pair2dl_t *result);

int naive_scan_points_lazy_has_next(
    struct lazy_handler_naive_scan_t *lazy_handler, int *result);

int naive_scan_points_lazy_reset(
    struct lazy_handler_naive_scan_t *lazy_handler);

int report_row_lazy_init(struct lazy_handler_report_band_t *lazy_handler,
                         struct block *input_block, struct queries_state *qs,
                         uint64_t coord);
int report_column_lazy_init(struct lazy_handler_report_band_t *lazy_handler,
                            struct block *input_block, struct queries_state *qs,
                            uint64_t coord);

int report_band_lazy_clean(struct lazy_handler_report_band_t *lazy_handler);
int report_band_next(struct lazy_handler_report_band_t *lazy_handler,
                     uint64_t *result);
int report_band_reset(struct lazy_handler_report_band_t *lazy_handler);

int report_band_has_next(struct lazy_handler_report_band_t *lazy_handler,
                         int *result);
```


# API: Mixed Tree

This tree's upper levels are a tree of pointers,
its leaves are the block's trees.

(From k2node.h)

```c
int k2node_has_point(struct k2node *k2node, unsigned long col, unsigned long row,
                     struct k2qstate *st, int *result);
int k2node_insert_point(struct k2node *input_node, unsigned long col, unsigned long row,
                        struct k2qstate *st, int *already_exists);
int k2node_delete_point(struct k2node *input_node, unsigned long col, unsigned long row,
                        struct k2qstate *st, int *already_not_exists);

int k2node_naive_scan_points(struct k2node *input_node, struct k2qstate *st,
                             struct vector_pair2dl_t *result);

int k2node_scan_points_interactively(struct k2node *input_node,
                                     struct k2qstate *st,
                                     point_reporter_fun_t point_reporter,
                                     void *report_state);

int k2node_report_column(struct k2node *input_node, unsigned long col,
                         struct k2qstate *st, struct vector_pair2dl_t *result);
int k2node_report_row(struct k2node *input_node, unsigned long row, struct k2qstate *st,
                      struct vector_pair2dl_t *result);

int k2node_report_column_interactively(struct k2node *input_node, unsigned long col,
                                       struct k2qstate *st,
                                       point_reporter_fun_t point_reporter,
                                       void *report_state);
int k2node_report_row_interactively(struct k2node *input_node, unsigned long row,
                                    struct k2qstate *st,
                                    point_reporter_fun_t point_reporter,
                                    void *report_state);

struct k2node *create_k2node(void);
int free_rec_k2node(struct k2node *input_node, unsigned long current_depth,
                    unsigned long cut_depth);

int init_k2qstate(struct k2qstate *st, TREE_DEPTH_T treedepth,
                  MAX_NODE_COUNT_T max_nodes_count, TREE_DEPTH_T cut_depth);
int clean_k2qstate(struct k2qstate *st);
struct k2tree_measurement k2node_measure_tree_size(struct k2node *input_node,
                                                   unsigned long cut_depth);

int k2node_naive_scan_points_lazy_init(
    struct k2node *input_node, struct k2qstate *st,
    struct k2node_lazy_handler_naive_scan_t *lazy_handler);

int k2node_naive_scan_points_lazy_clean(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler);

int k2node_naive_scan_points_lazy_next(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler, pair2dl_t *result);

int k2node_naive_scan_points_lazy_has_next(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler, int *result);

int k2node_naive_scan_points_lazy_reset(
    struct k2node_lazy_handler_naive_scan_t *lazy_handler);

int k2node_report_row_lazy_init(
    struct k2node_lazy_handler_report_band_t *lazy_handler,
    struct k2node *input_node, struct k2qstate *st, uint64_t coord);
int k2node_report_column_lazy_init(
    struct k2node_lazy_handler_report_band_t *lazy_handler,
    struct k2node *input_node, struct k2qstate *st, uint64_t coord);

int k2node_report_band_lazy_clean(
    struct k2node_lazy_handler_report_band_t *lazy_handler);
int k2node_report_band_next(
    struct k2node_lazy_handler_report_band_t *lazy_handler, uint64_t *result);

int k2node_report_band_has_next(
    struct k2node_lazy_handler_report_band_t *lazy_handler, int *result);

int k2node_report_band_reset(
    struct k2node_lazy_handler_report_band_t *lazy_handler);
```

# Some explanations

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

### More to be documented on k2node...

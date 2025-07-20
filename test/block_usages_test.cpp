//
// Created by cristobal on 29-04-20.
//

extern "C" {
#include "block/block.h"
#include "block-topology/block_topology.h"
#include "block-frontier/block_frontier.h"
#include "block/queries_state.h"
}

#include <iostream>
#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

bool comparePair(struct pair2dl a, struct pair2dl b){
  return a.col < b.col || (a.col == b.col && a.row < b.row);
}

void printVec(std::vector<struct pair2dl> &v){
  for(int i = 0; i < v.size(); i++){
    std::cout << v[i].col << ", " << v[i].row << "; ";
  }
  std::cout << std::endl;
}

TEST(usages, naive_scan_1){
  int err_code;
  uint32_t treedepth = 5;
  struct block *root_block = create_block(treedepth);

  struct queries_state qs;
  init_queries_state(&qs, treedepth);


  std::vector<struct pair2dl> init_elements = {{0, 0},  {3, 3},   {15, 3},
                                  {3, 15}, {30, 31}, {31, 8}};

  int qty = init_elements.size();
  for (int i = 0; i < qty; i++) {
    insert_point(root_block, init_elements[i].col, init_elements[i].row, &qs);
  }

  struct vector result;
  init_vector_with_capacity(&result, sizeof(struct pair2dl), qty);

  err_code = naive_scan_points(root_block, &qs, &result);
  if (err_code) {
    exit(err_code);
  }

  std::vector<struct pair2dl> store_points;
  for (int i = 0; i < result.nof_items; i++) {
    struct pair2dl *current;
    get_element_at(&result, i, (char **)&current);
    store_points.push_back(*current);
    // ASSERT_EQ(array_pairs[i].col, current->col) << "COL FAILED AT i = " << i;
    // ASSERT_EQ(array_pairs[i].row, current->row) << "ROW FAILED AT i = " << i;
  }

  std::sort(init_elements.begin(), init_elements.end(), comparePair);
  std::sort(store_points.begin(), store_points.end(), comparePair);

  printVec(init_elements);
  printVec(store_points);

  ASSERT_EQ(init_elements.size(), store_points.size()) << "DIFFERENT SIZE";

  for(int i = 0; i < init_elements.size(); i++){
    ASSERT_EQ(init_elements[i].col, store_points[i].col) << "(col) DIFFERENT AT i = " << i;
    ASSERT_EQ(init_elements[i].row, store_points[i].row) << "(row) DIFFERENT AT i = " << i;
  }

  free_vector(&result);

  err_code = free_rec_block(root_block);
  if (err_code)
    printf("There was an error with free_rec_block: %d\n", err_code);
  err_code = finish_queries_state(&qs);
  if (err_code)
    printf("There was an error with finish_queries_state: %d\n", err_code);

  if (!err_code) {
    printf("No errors while freeing data\n");
  }
}
//
// Created by Cristobal Miranda on 22-01-22.
//

#include <iostream>
#include <utility>
#include <vector>

#include <chrono>

extern "C" {
#include "block.h"
#include "k2node.h"
}

static void benchmark1(int size) {

  static constexpr unsigned long tree_depth = 32;

  struct block *root_block = create_block();

  struct queries_state qs;
  init_queries_state(&qs, tree_depth, 1024, root_block);

  auto start = std::chrono::high_resolution_clock::now();
  int point_exists;
  for (int i = 0; i < size; i++) {

    insert_point(root_block, i, i, &qs, &point_exists);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  auto microseconds = duration.count();
  std::cout << "Insertion Benchmark 1 took: " << microseconds
            << " microseconds, for input size = " << size
            << ", microsecs/point = " << (double)microseconds / (double)size
            << std::endl;

  start = std::chrono::high_resolution_clock::now();
  int count = 0;
  for (int i = 0; i < size; i++) {
    has_point(root_block, i, i, &qs, &point_exists);
    if (point_exists)
      count++;
  }
  end = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  microseconds = duration.count();

  std::cout << "Lookup Benchmark 1 took: " << microseconds
            << " microseconds, for input size = " << size
            << ", microsecs/point = " << (double)microseconds / (double)size
            << ", total count = " << count << std::endl;

  //  debug_print_block_tree_structure(root_block);

  finish_queries_state(&qs);
  free_rec_block(root_block);
}

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cerr << "missing size parameter" << std::endl;
    exit(1);
  }

  int size = std::stoi(argv[1]);

  benchmark1(size);
}
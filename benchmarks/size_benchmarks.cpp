#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

#include "fisher_yates.hpp"

extern "C" {
#include "block.h"
}

std::mt19937 rng(123321);

struct BenchmarkResult {
  int treedepth;
  int node_count;
  unsigned long points_count;
  unsigned long size_bytes;
  unsigned long bytes_ptrs;
  unsigned long bytes_topologies;
  unsigned long total_microseconds_inserting;
  unsigned long inserted_points;
};

BenchmarkResult space_benchmark_random_insertion_by_depth_and_node_count(
    TREE_DEPTH_T treedepth, MAX_NODE_COUNT_T node_count,
    unsigned long points_count, std::vector<unsigned long> &cols,
    std::vector<unsigned long> &rows);

int main(void) {

  unsigned long points_count = 1 << 25;
  unsigned long treedepth = 16;
  unsigned long side = 1 << treedepth;
  unsigned long side_count =
      std::min((unsigned long)std::sqrt(points_count), side);

  auto random_seq_1 = fisher_yates(side_count, side);
  auto random_seq_2 = fisher_yates(side_count, side);
  std::vector<BenchmarkResult> results;
  std::cout << "started experiments" << std::endl;
  for (int node_count_base = 6; node_count_base <= 10; node_count_base++) {

    results.push_back(space_benchmark_random_insertion_by_depth_and_node_count(
        treedepth, 1 << node_count_base, points_count, random_seq_1,
        random_seq_2));
  }

  std::cout << "Tree depth,Node count,Points count,Total Bytes,Bytes "
               "Ptrs+Preorders,Bytes Topologies,Total Time(Microsecs), Avg "
               "Time(Microsecs)"
            << std::endl;

  for (auto bm : results) {
    std::cout << bm.treedepth << "," << bm.node_count << ","
              << bm.inserted_points << "," << bm.size_bytes << ","
              << bm.bytes_ptrs << "," << bm.bytes_topologies << ","
              << bm.total_microseconds_inserting << ","
              << (float)bm.total_microseconds_inserting /
                     (float)bm.inserted_points
              << std::endl;
  }

  return 0;
}

BenchmarkResult space_benchmark_random_insertion_by_depth_and_node_count(
    TREE_DEPTH_T treedepth, MAX_NODE_COUNT_T node_count,
    unsigned long points_count, std::vector<unsigned long> &cols,
    std::vector<unsigned long> &rows) {
  struct block *root_block = create_block(static_cast<TREE_DEPTH_T>(treedepth));
  root_block->max_node_count = node_count;

  struct queries_state qs;
  init_queries_state(&qs, treedepth, root_block->max_node_count);

  // auto random_seq_1 = create_shuffled_sequence(points_count, side);
  // auto random_seq_2 = create_shuffled_sequence(points_count, side);

  /*
  for (size_t i = 0; i < points_count; i++) {
    insert_point(root_block, cols[i], rows[i], &qs);
  }
  */

  auto start = std::chrono::high_resolution_clock::now();
  unsigned long inserted_points = 0;
  for (size_t i = 0; i < cols.size(); i++) {
    if (i * rows.size() > points_count)
      break;
    for (size_t j = 0; j < rows.size(); j++) {
      if (i * rows.size() + j > points_count)
        break;
      insert_point(root_block, cols[i], rows[j], &qs);
      inserted_points++;
    }
  }
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  struct k2tree_measurement measurements = measure_tree_size(root_block);

  free_rec_block(root_block);
  finish_queries_state(&qs);

  return {treedepth,
          node_count,
          points_count,
          measurements.total_bytes,
          measurements.total_blocks *
              (sizeof(uint32_t) + sizeof(struct block *)),
          measurements.bytes_topology,
          (unsigned long)duration.count(),
          inserted_points};
}
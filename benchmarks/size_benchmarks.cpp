#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

extern "C" {
#include "block.h"
}

std::mt19937 rng(123321);

struct BenchmarkResult {
  int treedepth;
  int node_count;
  unsigned long points_count;
  unsigned long size_bytes;
};

std::vector<unsigned long> create_shuffled_sequence(unsigned long sequence_size,
                                                    unsigned long space_size);
BenchmarkResult space_benchmark_random_insertion_by_depth_and_node_count(
    TREE_DEPTH_T treedepth, MAX_NODE_COUNT_T node_count,
    unsigned long points_count);

int main(void) {

  std::vector<BenchmarkResult> results;
  for (int node_count_base = 7; node_count_base <= 10; node_count_base++) {

    results.push_back(space_benchmark_random_insertion_by_depth_and_node_count(
        25, 1 << node_count_base, 1 << 20));
  }

  std::cout << "Tree depth\t\tNode count\t\tPoints count\t\tBytes" << std::endl;

  for (auto bm : results) {
    std::cout << bm.treedepth << "\t\t\t" << bm.node_count << "\t\t\t"
              << bm.points_count << "\t\t\t" << bm.size_bytes << std::endl;
  }

  return 0;
}

std::vector<unsigned long> create_shuffled_sequence(unsigned long sequence_size,
                                                    unsigned long space_size) {
  std::vector<unsigned long> build_seq(space_size, 0);
  for (unsigned long i = 0; i < space_size; i++) {
    build_seq[i] = i;
  }

  std::random_shuffle(build_seq.begin(), build_seq.end());
  std::vector<unsigned long> out(sequence_size);

  std::copy(build_seq.begin(), build_seq.begin() + sequence_size, out.begin());

  return out;
}

BenchmarkResult space_benchmark_random_insertion_by_depth_and_node_count(
    TREE_DEPTH_T treedepth, MAX_NODE_COUNT_T node_count,
    unsigned long points_count) {
  unsigned long side = 1 << treedepth;
  struct block *root_block = create_block(static_cast<TREE_DEPTH_T>(treedepth));

  struct queries_state qs;
  init_queries_state(&qs, treedepth, root_block->max_node_count);

  auto random_seq_1 = create_shuffled_sequence(points_count, side);
  auto random_seq_2 = create_shuffled_sequence(points_count, side);

  for (size_t i = 0; i < points_count; i++) {
    insert_point(root_block, random_seq_1[i], random_seq_2[i], &qs);
  }

  unsigned long sz = measure_tree_size(root_block);

  free_rec_block(root_block);
  finish_queries_state(&qs);

  return {treedepth, node_count, points_count, sz};
}
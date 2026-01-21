extern "C" {
#include <block.h>
#include <queries_state.h>
}

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

std::mt19937 rng(123321);

void random_benchmark_by_depth(uint32_t treedepth, uint32_t points_count);
std::vector<unsigned long> create_shuffled_sequence(unsigned long sequence_size,
                                                    unsigned long space_size);

int main(void) {

  random_benchmark_by_depth(24, 1 << 20);
  random_benchmark_by_depth(28, 1 << 20);
  random_benchmark_by_depth(30, 1 << 20);
  random_benchmark_by_depth(32, 1 << 20);

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

void random_benchmark_by_depth(uint32_t treedepth, uint32_t points_count) {
  unsigned long side = 1 << treedepth;
  struct block *root_block = create_block(static_cast<TREE_DEPTH_T>(treedepth));

  struct queries_state qs;
  init_queries_state(&qs, treedepth);

  auto random_seq_1 = create_shuffled_sequence(points_count, side);
  auto random_seq_2 = create_shuffled_sequence(points_count, side);

  std::cout << "Started random_benchmark_by_depth with treedepth = "
            << treedepth << " and points_count = " << points_count << std::endl;
  auto start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < points_count; i++) {
    insert_point(root_block, random_seq_1[i], random_seq_2[i], &qs);
  }

  auto stop = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  std::cout << "Total Time in Microseconds: " << duration.count() << std::endl;
  std::cout << "Average Time inserting one point in Microseconds: "
            << duration.count() / points_count << std::endl;

  free_rec_block(root_block);
  finish_queries_state(&qs);
}
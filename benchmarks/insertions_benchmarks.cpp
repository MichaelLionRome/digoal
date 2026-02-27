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

  std::cout << "-------------------\n";
  std::cout << "Started random_benchmark_by_depth with treedepth = "
            << treedepth << " and points_count = " << points_count << std::endl;
  auto start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < points_count; i++) {
    insert_point(root_block, random_seq_1[i], random_seq_2[i], &qs);
  }

  auto stop = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  std::cout << "\n\nInsertion results\n";
  std::cout << "Total Time in Microseconds: " << duration.count() << std::endl;
  std::cout << "Average Time inserting one point in Microseconds: "
            << duration.count() / points_count << std::endl;

#ifdef DEBUG_STATS
  std::cout << "Time spent on sequential scan in total "
            << qs.dstats.time_on_sequential_scan << " microseconds"
            << std::endl;
  std::cout << "Time spent on sequential scan in average "
            << qs.dstats.time_on_sequential_scan / points_count
            << " microseconds" << std::endl;
  qs.dstats.time_on_sequential_scan = 0;
#endif

  start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < points_count; i++) {
    int has_point_result;
    has_point(root_block, random_seq_1[i], random_seq_2[i], &qs,
              &has_point_result);
    if (!has_point_result) {
      std::cerr << "Point " << i << " not found (" << random_seq_1[i] << ", "
                << random_seq_2[i] << ")" << std::endl;
      exit(1);
    }
  }
  stop = std::chrono::high_resolution_clock::now();

  duration =
      std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  std::cout << "\n\nSuccesful Querying Results\n";
  std::cout << "Total Time in Microseconds: " << duration.count() << std::endl;
  std::cout << "Average Time querying one point in Microseconds: "
            << duration.count() / points_count << std::endl;
#ifdef DEBUG_STATS
  std::cout << "Time spent on sequential scan in total "
            << qs.dstats.time_on_sequential_scan << " microseconds"
            << std::endl;
  std::cout << "Time spent on sequential scan in average "
            << qs.dstats.time_on_sequential_scan / points_count
            << " microseconds" << std::endl;
  qs.dstats.time_on_sequential_scan = 0;
#endif

  std::cout << "-------------------\n\n\n" << std::endl;

  free_rec_block(root_block);
  finish_queries_state(&qs);
}
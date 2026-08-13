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
#ifndef _FISHER_YATES_H
#define _FISHER_YATES_H

#include <random>
#include <unordered_map>
#include <vector>

std::vector<unsigned long> fisher_yates(unsigned long result_size,
                                        unsigned long choice_set_size) {
  std::vector<unsigned long> result;
  std::unordered_map<unsigned long, unsigned long> state;

  for (unsigned long i = 0; i < result_size; i++) {
    unsigned long random_number = (std::rand() % (choice_set_size - i)) + i;
    unsigned long which_rand = random_number;
    unsigned long which_i = i;
    if (state.find(random_number) != state.end())
      which_rand = state[random_number];

    if (state.find(i) != state.end())
      which_i = state[i];

    state[i] = which_rand;
    state[random_number] = which_i;
  }

  for (unsigned long i = 0; i < result_size; i++)
    result.push_back(state[i] + 1);

  return result;
}
#endif

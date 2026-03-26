cmake_minimum_required(VERSION 3.14)
project(k2tree-dyn)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror")
set(CMAKE_C_FLAGS "-Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Werror")
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG_STATS")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17 -pedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

option(WITH_DEBUG_STATS "Debug stats" OFF)

if(WITH_DEBUG_STATS)
add_definitions(-DDEBUG_STATS)
endif()

set(SOURCES_REQUIRED
src/block.c
src/block_frontier.c
src/block_topology.c
src/custom_bv_handling.c
src/morton_code.c
src/queries_state.c
src/stacks.c
src/vectors.c
)

set(SOURCES_MEM_DEFAULT
src/default_memalloc.c
)

set(SOURCES 
${SOURCES_REQUIRED}
${SOURCES_MEM_DEFAULT}
)

set(LIB_INCLUDES
lib/c-bitvector/include
lib/c-queue/include
lib/c-vector/include
)

set(LIB_SOURCES
lib/c-bitvector/src/bitvector.c
)


include_directories(include ${LIB_INCLUDES})

add_library(k2dyn ${SOURCES} ${LIB_SOURCES})
add_library(k2dyn_noalloc ${SOURCES_REQUIRED} ${LIB_SOURCES})
target_link_libraries(k2dyn m)
target_link_libraries(k2dyn_noalloc m)

add_executable(example1 example/example1.c)
add_executable(example2 example/example2.c)
add_executable(example3 example/example3.c)
add_executable(example4 example/example4.c)
add_executable(example5 example/example5.c)

target_link_libraries(example1 k2dyn)
target_link_libraries(example2 k2dyn)
target_link_libraries(example3 k2dyn)
target_link_libraries(example4 k2dyn)
target_link_libraries(example5 k2dyn)



add_executable(insertions_benchmarks benchmarks/insertions_benchmarks.cpp)
target_link_libraries(insertions_benchmarks k2dyn)



find_package(GTest QUIET)
if(GTest_FOUND)

enable_testing()

include_directories(${GTEST_INCLUDE_DIRS})
add_executable(block_test test/block_test.cpp test/block_wrapper.hpp)
add_executable(block_leak_test test/block_leak_test.cpp test/block_wrapper.hpp)
add_executable(morton_code_test test/morton_code_test.cpp)
add_executable(block_usages_test test/block_usages_test.cpp)
add_executable(debug_insertion_out_of_bounds_1_test test/debug_insertion_out_of_bounds_1_test.cpp)
add_executable(interactive_report_test test/interactive_report_test.cpp)


target_link_libraries(block_test  ${GTEST_BOTH_LIBRARIES} pthread k2dyn)
target_link_libraries(block_leak_test  ${GTEST_BOTH_LIBRARIES} pthread k2dyn)
target_link_libraries(morton_code_test  ${GTEST_BOTH_LIBRARIES} pthread k2dyn )
target_link_libraries(block_usages_test   ${GTEST_BOTH_LIBRARIES} pthread k2dyn )
target_link_libraries(debug_insertion_out_of_bounds_1_test   k2dyn  ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(interactive_report_test   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)


add_test(NAME block_test COMMAND ./block_test)
add_test(NAME block_leak_test COMMAND ./block_leak_test)
add_test(NAME morton_code_test COMMAND ./morton_code_test)
add_test(NAME block_usages_test COMMAND ./block_usages_test)
add_test(NAME interactive_report_test COMMAND ./interactive_report_test)

endif()
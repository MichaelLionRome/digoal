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
#set(CMAKE_C_FLAGS "-Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition")
set(CMAKE_C_FLAGS_DEBUG "-g")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17 -pedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#set(CMAKE_LINKER_FLAGS_DEBUG "-fsanitize=address")
option(WITH_DEBUG_STATS "Debug stats" OFF)

if(WITH_DEBUG_STATS)
add_definitions(-DDEBUG_STATS)
endif()

add_definitions(-DLIGHT_FIELDS)

set(SOURCES_REQUIRED
src/block.c
src/block_frontier.c
src/block_topology.c
src/custom_bv_handling.c
src/morton_code.c
src/queries_state.c
src/stacks.c
src/vectors.c
src/bitvector.c
src/k2node.c
)

set(SOURCES_MEM_DEFAULT
src/default_memalloc.c
)

set(SOURCES 
${SOURCES_REQUIRED}
${SOURCES_MEM_DEFAULT}
)

include_directories(include)

add_library(k2dyn ${SOURCES})
target_link_libraries(k2dyn m)

add_executable(example1 example/example1.c)
add_executable(example2 example/example2.c)
add_executable(example3 example/example3.c)
add_executable(example4 example/example4.c)
add_executable(example5 example/example5.c)
add_executable(example6 example/example6.c)
add_executable(example7 example/example7.c)

target_link_libraries(example1 k2dyn)
target_link_libraries(example2 k2dyn)
target_link_libraries(example3 k2dyn)
target_link_libraries(example4 k2dyn)
target_link_libraries(example5 k2dyn)
target_link_libraries(example6 k2dyn)
target_link_libraries(example7 k2dyn)



add_executable(insertions_benchmarks benchmarks/insertions_benchmarks.cpp)
target_link_libraries(insertions_benchmarks k2dyn)


add_executable(size_benchmarks benchmarks/size_benchmarks.cpp)
target_link_libraries(size_benchmarks k2dyn)



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
add_executable(bitvector_test test/bitvector_test.cpp)
add_executable(k2node_test test/k2node_test.cpp)
add_executable(sip_join_test test/sip_join_test.cpp)
add_executable(k2node_sip_join_test test/k2node_sip_join_test.cpp)
add_executable(lazy_scan_test test/lazy_scan_test.cpp)
add_executable(block_delete_test test/block_delete_test.cpp)
add_executable(k2node_delete_test test/k2node_delete_test.cpp)
add_executable(block_small_tests test/block_small_tests.cpp)
add_executable(k2node_small_tests test/k2node_small_tests.cpp)
add_executable(k2node_problematic_input_tests test/k2node_problematic_input_tests.cpp)

target_link_libraries(block_test  ${GTEST_BOTH_LIBRARIES} pthread k2dyn)
target_link_libraries(block_leak_test  ${GTEST_BOTH_LIBRARIES} pthread k2dyn)
target_link_libraries(morton_code_test  ${GTEST_BOTH_LIBRARIES} pthread k2dyn )
target_link_libraries(block_usages_test   ${GTEST_BOTH_LIBRARIES} pthread k2dyn )
target_link_libraries(debug_insertion_out_of_bounds_1_test   k2dyn  ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(interactive_report_test   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(bitvector_test   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(k2node_test   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(sip_join_test   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(k2node_sip_join_test   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(lazy_scan_test   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(block_delete_test   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(k2node_delete_test   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(block_small_tests   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(k2node_small_tests   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(k2node_problematic_input_tests   k2dyn ${GTEST_BOTH_LIBRARIES} pthread)


add_test(NAME block_test COMMAND ./block_test)
add_test(NAME block_leak_test COMMAND ./block_leak_test)
add_test(NAME morton_code_test COMMAND ./morton_code_test)
add_test(NAME block_usages_test COMMAND ./block_usages_test)
add_test(NAME interactive_report_test COMMAND ./interactive_report_test)
add_test(NAME k2node_test COMMAND ./k2node_test)
add_test(NAME sip_join_test COMMAND ./sip_join_test)
add_test(NAME k2node_sip_join_test COMMAND ./k2node_sip_join_test)
add_test(NAME lazy_scan_test COMMAND ./lazy_scan_test)
add_test(NAME block_delete_test COMMAND ./block_delete_test)
add_test(NAME k2node_delete_test COMMAND ./block_delete_test)
add_test(NAME block_small_tests COMMAND ./block_small_tests)
add_test(NAME k2node_small_tests COMMAND ./k2node_small_tests)
add_test(NAME k2node_problematic_input_tests COMMAND ./k2node_problematic_input_tests)

endif()
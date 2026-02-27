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
src/definitions.c
src/morton_code.c
src/queries_state.c
src/int_stack.c
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
lib/c-queue/src/circular_queue.c
lib/c-vector/src/vector.c
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

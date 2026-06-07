cmake_minimum_required(VERSION 3.14)
project(c-bitvector)
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

include_directories(include)

add_library(bitvector src/bitvector.c)

find_package(GTest QUIET)
if(GTest_FOUND)

enable_testing()

add_executable(bitvector_test test/bitvector_test.cpp test/bitvector_wrapper.h)
target_link_libraries(bitvector_test bitvector ${GTEST_BOTH_LIBRARIES} pthread )


add_test(NAME bitvector_test COMMAND ./bitvector_test)

endif()
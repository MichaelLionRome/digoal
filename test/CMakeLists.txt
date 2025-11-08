cmake_minimum_required(VERSION 3.14)
project(K2TreeElementsTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(CMAKE_BUILD_TYPE Debug)


set(ROOT_PATH ..)
set(BITVECTOR_PATH ${ROOT_PATH}/lib/c-bitvector)
set(VECTOR_PATH ${ROOT_PATH}/lib/c-vector)
set(CIRCULAR_QUEUE_PATH ${ROOT_PATH}/lib/c-queue)

set(BITVECTOR_INCLUDE ${BITVECTOR_PATH}/include)
set(VECTOR_INCLUDE ${VECTOR_PATH}/include)
set(CIRCULAR_QUEUE_INCLUDE ${CIRCULAR_QUEUE_PATH}/include)
set(K2TREE_INCLUDES ${ROOT_PATH}/include)

include_directories(${BITVECTOR_INCLUDE} ${VECTOR_INCLUDE} ${CIRCULAR_QUEUE_INCLUDE} ${K2TREE_INCLUDES})
link_directories(${BITVECTOR_PATH}/bin ${VECTOR_PATH}/bin ${CIRCULAR_QUEUE_PATH}/bin)

set(K2TreePath ../src)

set(INCLUDES_ELEMENTS ${K2TREE_INCLUDES}/block.h 
 ${K2TREE_INCLUDES}/queries_state.h
 ${K2TREE_INCLUDES}/block_frontier.h
 ${K2TREE_INCLUDES}/block_topology.h
 ${K2TREE_INCLUDES}/morton_code.h
 ${K2TREE_INCLUDES}/definitions.h
 ${K2TREE_INCLUDES}/memalloc.h
 ${K2TREE_INCLUDES}/custom_bv_handling.h
 )

set(SOURCES_ELEMENTS ${K2TreePath}/block.c
 ${K2TreePath}/queries_state.c
 ${K2TreePath}/block_frontier.c
 ${K2TreePath}/block_topology.c
 ${K2TreePath}/morton_code.c
 ${K2TreePath}/definitions.c
 ${K2TreePath}/default_memalloc.c
 ${K2TreePath}/custom_bv_handling.c
 )

add_library(K2TreeElements ${SOURCES_ELEMENTS})

## TESTING

### GOOGLE TEST CONFIG BEGIN
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

# Now simply link against gtest or gtest_main as needed. Eg
#add_executable(example example.cpp)
#target_link_libraries(example gtest_main)
#add_test(NAME example_test COMMAND example)
### GOOGLE TEST CONFIG END

set(TEST_FILES
        block_test.cpp)

add_executable(block_test block_test.cpp block_wrapper.hpp)
add_executable(block_leak_test block_leak_test.cpp block_wrapper.hpp)
add_executable(morton_code_test morton_code_test.cpp)
add_executable(block_usages_test block_usages_test.cpp)
add_executable(debug_insertion_out_of_bounds_1_test debug_insertion_out_of_bounds_1_test.cpp)


target_link_libraries(block_test K2TreeElements gtest_main vector circular_queue bitvector)
target_link_libraries(block_leak_test K2TreeElements vector circular_queue bitvector)
target_link_libraries(morton_code_test K2TreeElements vector circular_queue bitvector gtest_main)
target_link_libraries(block_usages_test K2TreeElements vector circular_queue bitvector gtest_main)
target_link_libraries(debug_insertion_out_of_bounds_1_test K2TreeElements vector circular_queue bitvector gtest_main)

cmake_minimum_required(VERSION 3.14)
project(K2TreeElementsTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(ROOT_PATH ..)
set(BITVECTOR_PATH ${ROOT_PATH}/lib/c-bitvector)
set(VECTOR_PATH ${ROOT_PATH}/lib/c-vector)
set(CIRCULAR_QUEUE_PATH ${ROOT_PATH}/lib/c-queue)

set(BITVECTOR_INCLUDE ${BITVECTOR_PATH}/include)
set(VECTOR_INCLUDE ${VECTOR_PATH}/include)
set(CIRCULAR_QUEUE_INCLUDE ${CIRCULAR_QUEUE_PATH}/include)
set(K2TREE_INCLUDES ${ROOT_PATH}/k2tree)

include_directories(../k2tree ../k2tree/block ../k2tree/block-frontier ../k2tree/block-topology ../k2tree/morton-code)
include_directories(${BITVECTOR_INCLUDE} ${VECTOR_INCLUDE} ${CIRCULAR_QUEUE_INCLUDE} ${K2TREE_INCLUDES})
link_directories(${BITVECTOR_PATH}/bin ${VECTOR_PATH}/bin ${CIRCULAR_QUEUE_PATH}/bin)

set(K2TreePath ../k2tree)
set(BlockPath ${K2TreePath}/block)
set(BlockFrontierPath ${K2TreePath}/block-frontier)
set(BlockTopologyPath ${K2TreePath}/block-topology)
set(MortonCodePath ${K2TreePath}/morton-code)

set(INCLUDES_ELEMENTS ${BlockPath}/block.h 
 ${BlockPath}/queries_state.h
 ${BlockFrontierPath}/block_frontier.h
 ${BlockTopologyPath}/block_topology.h
 ${MortonCodePath}/morton_code.h
 ${K2TREE_INCLUDES}/definitions.h
 )

set(SOURCES_ELEMENTS ${BlockPath}/block.c
 ${BlockPath}/queries_state.c
 ${BlockFrontierPath}/block_frontier.c
 ${BlockTopologyPath}/block_topology.c
 ${MortonCodePath}/morton_code.c
 ${K2TREE_INCLUDES}/definitions.c
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
target_link_libraries(block_test K2TreeElements gtest_main vector circular_queue bitvector)

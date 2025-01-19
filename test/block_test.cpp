#include <gtest/gtest.h>

extern "C" {
#include "block/block.h"
#include "block-topology/block_topology.h"
#include "block-frontier/block_frontier.h"
#include "block/queries_state.h"
}

TEST(block_test, test1){
    uint32_t tree_depth = 3;
    struct block *root_block = create_block(tree_depth);
    struct queries_state qs;
    init_queries_state(&qs, tree_depth);


    insert_point(root_block, 0, 0, &qs);
    int found_point;
    has_point(root_block, 0, 0, &qs, &found_point);
    ASSERT_EQ(1, found_point);

    free_block(root_block);
    finish_queries_state(&qs);
}
#include "dfo_rpo.h"
#include "irGen.h"
#include "gtest/gtest.h"

namespace ir::tests {

class dfoRpoTest : public ::testing::Test {

  public:
    virtual void SetUp() { irGenerator.CreateGraph(); }
    dfoRpoTest() = default;
    virtual void TearDown() { irGenerator.Clear(); }
    IRGenerator irGenerator;
};

TEST_F(dfoRpoTest, TestDFO) {
    auto *blockA = irGenerator.CreateEmptyBB();
    auto *blockB = irGenerator.CreateEmptyBB();
    auto *blockC = irGenerator.CreateEmptyBB();
    auto *blockD = irGenerator.CreateEmptyBB();
    auto *graph = irGenerator.GetGraph();
    graph->SetFirstBB(blockA);
    graph->ConnectBBs(blockA, blockB);
    graph->ConnectBBs(blockA, blockC);
    graph->ConnectBBs(blockB, blockD);
    graph->ConnectBBs(blockC, blockD);
    std::vector<BB *> bblocks;
    bblocks.reserve(4);
    // do depth-first order traversal
    auto dfo = DFO();
    dfo.ValidateGraph(graph,
                      [&bblocks](BB *bblock) { bblocks.push_back(bblock); });
    ASSERT_EQ(bblocks.size(), 4);
    ASSERT_EQ(bblocks[0], blockD);
    ASSERT_TRUE(bblocks[1] == blockB || bblocks[1] == blockC);
    ASSERT_TRUE(bblocks[2] == blockB || bblocks[2] == blockC);
    ASSERT_EQ(bblocks[3], blockA);
}

TEST_F(dfoRpoTest, TestRPO) {
    // create graph
    auto *blockA = irGenerator.CreateEmptyBB();
    auto *blockB = irGenerator.CreateEmptyBB();
    auto *blockC = irGenerator.CreateEmptyBB();
    auto *blockD = irGenerator.CreateEmptyBB();
    auto *graph = irGenerator.GetGraph();
    graph->SetFirstBB(blockA);
    graph->ConnectBBs(blockA, blockB);
    graph->ConnectBBs(blockA, blockC);
    graph->ConnectBBs(blockB, blockD);
    graph->ConnectBBs(blockC, blockD);
    // do reverse post-order traversal
    auto bblocks = RPO(graph);
    ASSERT_EQ(bblocks.size(), 4);
    ASSERT_EQ(bblocks[0], blockA);
    ASSERT_TRUE(bblocks[1] == blockB || bblocks[1] == blockC);
    ASSERT_TRUE(bblocks[2] == blockB || bblocks[2] == blockC);
    ASSERT_EQ(bblocks[3], blockD);
    // change layout and do RPO once again
    graph->ConnectBBs(blockD, blockA);
    auto bblocks2 = RPO(graph);
    ASSERT_EQ(bblocks.size(), bblocks2.size());
    for (size_t i = 0; i < bblocks.size(); ++i) {
        ASSERT_EQ(bblocks[i], bblocks2[i]);
    }
}
} // namespace ir::tests
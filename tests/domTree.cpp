#include "domTree.h"
#include "testBase.h"
#include <iostream>

namespace ir::tests {
class DomTreeTest : public TestBase {};
static void checkDominatedBBs(BB *bblock, const std::vector<BB *> &expected) {
    ASSERT_NE(bblock, nullptr);
    auto dominated = bblock->GetDominatedBBs();
    ASSERT_EQ(dominated.size(), expected.size());
    if (expected.empty()) {
        return;
    }
    std::sort(dominated.begin(), dominated.end(),
              [](BB *lhs, BB *rhs) { return lhs->GetId() < rhs->GetId(); });
    for (size_t i = 0; i < dominated.size(); ++i) {
        ASSERT_EQ(dominated[i], expected[i]);
    }
}

TEST_F(DomTreeTest, TestBuilding1) {
    std::vector<BB *> bblocks(7);
    for (auto &it : bblocks) {
        it = GetGraph()->CreateEmptyBB();
    }
    auto *graph = GetGraph();
    graph->SetFirstBB(bblocks[0]);
    graph->ConnectBBs(bblocks[0], bblocks[1]);
    graph->ConnectBBs(bblocks[1], bblocks[2]);
    graph->ConnectBBs(bblocks[1], bblocks[5]);
    graph->ConnectBBs(bblocks[2], bblocks[3]);
    graph->ConnectBBs(bblocks[4], bblocks[3]);
    graph->ConnectBBs(bblocks[5], bblocks[4]);
    graph->ConnectBBs(bblocks[5], bblocks[6]);
    graph->ConnectBBs(bblocks[6], bblocks[3]);
    DomTreeBuilder domTreeBuilder;
    domTreeBuilder.Construct(graph);
    // check dominators
    std::vector<BB *> expectedDominators{nullptr,    bblocks[0], bblocks[1],
                                         bblocks[1], bblocks[5], bblocks[1],
                                         bblocks[5]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        ASSERT_EQ(bblocks[i]->GetDominator(), expectedDominators[i]);
    }
    // check dominated basic blocks
    std::vector<std::vector<BB *>> expectedDominatedBlocks(bblocks.size());
    expectedDominatedBlocks[0] = {bblocks[1]};
    expectedDominatedBlocks[1] = {bblocks[2], bblocks[3], bblocks[5]};
    expectedDominatedBlocks[5] = {bblocks[4], bblocks[6]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        checkDominatedBBs(bblocks[i], expectedDominatedBlocks[i]);
    }
}

TEST_F(DomTreeTest, TestBuilding2) {
    std::vector<BB *> bblocks(11);
    for (auto &it : bblocks) {
        it = GetGraph()->CreateEmptyBB();
    }
    auto *graph = GetGraph();
    graph->SetFirstBB(bblocks[0]);
    for (size_t i = 0; i < 7; ++i) {
        graph->ConnectBBs(bblocks[i], bblocks[i + 1]);
    }
    graph->ConnectBBs(bblocks[3], bblocks[2]);
    graph->ConnectBBs(bblocks[5], bblocks[4]);
    graph->ConnectBBs(bblocks[7], bblocks[1]);
    graph->ConnectBBs(bblocks[1], bblocks[9]);
    graph->ConnectBBs(bblocks[9], bblocks[2]);
    graph->ConnectBBs(bblocks[6], bblocks[8]);
    graph->ConnectBBs(bblocks[8], bblocks[10]);
    DomTreeBuilder domTreeBuilder;
    domTreeBuilder.Construct(graph);
    // check dominators
    std::vector<BB *> expectedDominators{
        nullptr,    bblocks[0], bblocks[1], bblocks[2], bblocks[3], bblocks[4],
        bblocks[5], bblocks[6], bblocks[6], bblocks[1], bblocks[8]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        ASSERT_EQ(bblocks[i]->GetDominator(), expectedDominators[i]);
    }
    // check dominated basic blocks
    std::vector<std::vector<BB *>> expectedDominatedBlocks(bblocks.size());
    for (size_t i = 2; i < 6; ++i) {
        expectedDominatedBlocks[i] = {bblocks[i + 1]};
    }
    expectedDominatedBlocks[0] = {bblocks[1]};
    expectedDominatedBlocks[1] = {bblocks[2], bblocks[9]};
    expectedDominatedBlocks[6] = {bblocks[7], bblocks[8]};
    expectedDominatedBlocks[8] = {bblocks[10]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        checkDominatedBBs(bblocks[i], expectedDominatedBlocks[i]);
    }
}

TEST_F(DomTreeTest, TestBuilding3) {
    std::vector<BB *> bblocks(9);
    for (auto &it : bblocks) {
        it = GetGraph()->CreateEmptyBB();
    }
    auto *graph = GetGraph();
    graph->SetFirstBB(bblocks[0]);
    for (size_t i = 0; i < 3; ++i) {
        graph->ConnectBBs(bblocks[i], bblocks[i + 1]);
    }
    graph->ConnectBBs(bblocks[1], bblocks[4]);
    graph->ConnectBBs(bblocks[4], bblocks[3]);
    graph->ConnectBBs(bblocks[4], bblocks[5]);
    graph->ConnectBBs(bblocks[5], bblocks[7]);
    graph->ConnectBBs(bblocks[5], bblocks[1]);
    graph->ConnectBBs(bblocks[7], bblocks[6]);
    graph->ConnectBBs(bblocks[7], bblocks[8]);
    graph->ConnectBBs(bblocks[3], bblocks[6]);
    graph->ConnectBBs(bblocks[6], bblocks[2]);
    graph->ConnectBBs(bblocks[6], bblocks[8]);
    DomTreeBuilder domTreeBuilder;
    domTreeBuilder.Construct(graph);
    // check dominators
    std::vector<BB *> expectedDominators{nullptr,    bblocks[0], bblocks[1],
                                         bblocks[1], bblocks[1], bblocks[4],
                                         bblocks[1], bblocks[5], bblocks[1]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        ASSERT_EQ(bblocks[i]->GetDominator(), expectedDominators[i]);
    }
    // check dominated basic blocks
    std::vector<std::vector<BB *>> expectedDominatedBlocks(bblocks.size());
    expectedDominatedBlocks[0] = {bblocks[1]};
    expectedDominatedBlocks[1] = {bblocks[2], bblocks[3], bblocks[4],
                                  bblocks[6], bblocks[8]};
    expectedDominatedBlocks[4] = {bblocks[5]};
    expectedDominatedBlocks[5] = {bblocks[7]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        checkDominatedBBs(bblocks[i], expectedDominatedBlocks[i]);
    }
}
} // namespace ir::tests
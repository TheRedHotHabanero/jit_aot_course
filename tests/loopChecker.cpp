#include "loopChecker.h"
#include "testBase.h"
#include <iostream>

namespace ir::tests {
class LoopAnalysisTest : public TestBase {
  public:
    LoopChecker loopChecker;
};

TEST_F(LoopAnalysisTest, TestLoops1) {
    std::vector<BB *> bblocks(7);
    auto *graph = GetGraph();
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBB();
    }
    graph->SetFirstBB(bblocks[0]);
    graph->ConnectBBs(bblocks[0], bblocks[1]);
    graph->ConnectBBs(bblocks[1], bblocks[2]);
    graph->ConnectBBs(bblocks[1], bblocks[5]);
    graph->ConnectBBs(bblocks[2], bblocks[3]);
    graph->ConnectBBs(bblocks[4], bblocks[3]);
    graph->ConnectBBs(bblocks[5], bblocks[4]);
    graph->ConnectBBs(bblocks[5], bblocks[6]);
    graph->ConnectBBs(bblocks[6], bblocks[3]);

    loopChecker.VerifyGraphLoops(graph);
    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    ASSERT_TRUE(rootLoop->GetInnerLoops().empty());
    ASSERT_EQ(rootLoop->GetBasicBlocks().size(), graph->GetBBCount());
    for (auto *bblock : bblocks) {
        ASSERT_EQ(bblock->GetLoop(), rootLoop);
    }
}

TEST_F(LoopAnalysisTest, TestLoops2) {
    std::vector<BB *> bblocks(11);
    auto *graph = GetGraph();
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBB();
    }
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

    loopChecker.VerifyGraphLoops(graph);
    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    auto innerLoops = rootLoop->GetInnerLoops();
    ASSERT_EQ(innerLoops.size(), 1);
    auto *mainLoop = innerLoops[0];
    ASSERT_EQ(mainLoop->GetOuterLoop(), rootLoop);
    ASSERT_FALSE(mainLoop->IsRoot());
    ASSERT_EQ(mainLoop->GetHeader(), bblocks[1]);
    ASSERT_EQ(mainLoop->GetInnerLoops().size(), 2);
    ASSERT_EQ(mainLoop->GetBackEdges().size(), 1);
    ASSERT_EQ(mainLoop->GetBackEdges()[0], bblocks[7]);
    ASSERT_EQ(mainLoop->GetBasicBlocks().size(), 4);
    ASSERT_FALSE(mainLoop->IsIrreducible());
    for (auto *loop : mainLoop->GetInnerLoops()) {
        ASSERT_EQ(loop->GetOuterLoop(), mainLoop);
        ASSERT_FALSE(loop->IsRoot());
        ASSERT_TRUE(loop->GetInnerLoops().empty());
        ASSERT_EQ(loop->GetBasicBlocks().size(), 2);
        ASSERT_EQ(loop->GetBackEdges().size(), 1);
        if (loop->GetHeader() == bblocks[2]) {
            ASSERT_EQ(loop->GetBackEdges()[0], bblocks[3]);
        } else {
            ASSERT_EQ(loop->GetHeader(), bblocks[4]);
            ASSERT_EQ(loop->GetBackEdges()[0], bblocks[5]);
        }
        ASSERT_FALSE(loop->IsIrreducible());
    }
}

TEST_F(LoopAnalysisTest, TestLoops3) {
    std::vector<BB *> bblocks(9);
    auto *graph = GetGraph();
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBB();
    }
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

    loopChecker.VerifyGraphLoops(graph);
    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    ASSERT_EQ(rootLoop->GetInnerLoops().size(), 2);
    for (auto *loop : rootLoop->GetInnerLoops()) {
        ASSERT_FALSE(loop->IsRoot());
        ASSERT_EQ(loop->GetOuterLoop(), rootLoop);
        ASSERT_TRUE(loop->GetInnerLoops().empty());
        ASSERT_EQ(loop->GetBackEdges().size(), 1);
        if (loop->GetHeader() == bblocks[1]) {
            ASSERT_FALSE(loop->IsIrreducible());
            ASSERT_EQ(loop->GetBackEdges()[0], bblocks[5]);
            ASSERT_EQ(loop->GetBasicBlocks().size(), 3);
        } else {
            ASSERT_EQ(loop->GetHeader(), bblocks[2]);
            ASSERT_TRUE(loop->IsIrreducible());
            ASSERT_EQ(loop->GetBackEdges()[0], bblocks[6]);
            ASSERT_EQ(loop->GetBasicBlocks().size(), 1);
        }
    }
}

TEST_F(LoopAnalysisTest, TestLoops4) {
    std::vector<BB *> bblocks(5);
    auto *graph = GetGraph();
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBB();
    }
    graph->SetFirstBB(bblocks[0]);
    graph->ConnectBBs(bblocks[0], bblocks[1]);
    graph->ConnectBBs(bblocks[1], bblocks[2]);
    graph->ConnectBBs(bblocks[1], bblocks[3]);
    graph->ConnectBBs(bblocks[3], bblocks[4]);
    graph->ConnectBBs(bblocks[4], bblocks[1]);
    loopChecker.VerifyGraphLoops(graph);
    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    ASSERT_EQ(rootLoop->GetBasicBlocks().size(), 2);
    ASSERT_EQ(rootLoop->GetInnerLoops().size(), 1);
    auto *loop = rootLoop->GetInnerLoops()[0];
    ASSERT_EQ(loop->GetOuterLoop(), rootLoop);
    ASSERT_FALSE(loop->IsRoot());
    ASSERT_EQ(loop->GetHeader(), bblocks[1]);
    ASSERT_TRUE(loop->GetInnerLoops().empty());
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges()[0], bblocks[4]);
    ASSERT_EQ(loop->GetBasicBlocks().size(), 3);
    ASSERT_FALSE(loop->IsIrreducible());
}

TEST_F(LoopAnalysisTest, TestLoops5) {
    std::vector<BB *> bblocks(6);
    for (auto &it : bblocks) {
        it = GetGraph()->CreateEmptyBB();
    }
    auto *graph = GetGraph();
    graph->SetFirstBB(bblocks[0]);
    for (size_t i = 0; i < 3; ++i) {
        graph->ConnectBBs(bblocks[i], bblocks[i + 1]);
    }
    graph->ConnectBBs(bblocks[2], bblocks[4]);
    graph->ConnectBBs(bblocks[4], bblocks[3]);
    graph->ConnectBBs(bblocks[4], bblocks[5]);
    graph->ConnectBBs(bblocks[5], bblocks[1]);
    loopChecker.VerifyGraphLoops(graph);
    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    ASSERT_EQ(rootLoop->GetBasicBlocks().size(), 2);
    ASSERT_EQ(rootLoop->GetInnerLoops().size(), 1);
    auto *loop = rootLoop->GetInnerLoops()[0];
    ASSERT_EQ(loop->GetOuterLoop(), rootLoop);
    ASSERT_FALSE(loop->IsRoot());
    ASSERT_EQ(loop->GetHeader(), bblocks[1]);
    ASSERT_TRUE(loop->GetInnerLoops().empty());
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges()[0], bblocks[5]);
    ASSERT_EQ(loop->GetBasicBlocks().size(), 4);
    ASSERT_FALSE(loop->IsIrreducible());
}

TEST_F(LoopAnalysisTest, TestLoops6) {
    std::vector<BB *> bblocks(8);
    for (auto &it : bblocks) {
        it = GetGraph()->CreateEmptyBB();
    }
    auto *graph = GetGraph();
    graph->SetFirstBB(bblocks[0]);
    graph->ConnectBBs(bblocks[0], bblocks[1]);
    graph->ConnectBBs(bblocks[1], bblocks[2]);
    graph->ConnectBBs(bblocks[1], bblocks[3]);
    graph->ConnectBBs(bblocks[2], bblocks[4]);
    graph->ConnectBBs(bblocks[2], bblocks[5]);
    graph->ConnectBBs(bblocks[3], bblocks[5]);
    graph->ConnectBBs(bblocks[5], bblocks[6]);
    graph->ConnectBBs(bblocks[6], bblocks[7]);
    graph->ConnectBBs(bblocks[6], bblocks[1]);
    graph->ConnectBBs(bblocks[7], bblocks[0]);
    loopChecker.VerifyGraphLoops(graph);
    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    ASSERT_EQ(rootLoop->GetBasicBlocks().size(), 1);
    ASSERT_EQ(rootLoop->GetInnerLoops().size(), 1);
    auto *loop = rootLoop->GetInnerLoops()[0];
    ASSERT_EQ(loop->GetOuterLoop(), rootLoop);
    ASSERT_FALSE(loop->IsRoot());
    ASSERT_EQ(loop->GetHeader(), bblocks[0]);
    ASSERT_EQ(loop->GetInnerLoops().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges()[0], bblocks[7]);
    ASSERT_EQ(loop->GetBasicBlocks().size(), 2);
    ASSERT_FALSE(loop->IsIrreducible());
    loop = loop->GetInnerLoops()[0];
    ASSERT_EQ(loop->GetOuterLoop(), rootLoop->GetInnerLoops()[0]);
    ASSERT_FALSE(loop->IsRoot());
    ASSERT_EQ(loop->GetHeader(), bblocks[1]);
    ASSERT_TRUE(loop->GetInnerLoops().empty());
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges()[0], bblocks[6]);
    ASSERT_EQ(loop->GetBasicBlocks().size(), 5);
    ASSERT_FALSE(loop->IsIrreducible());
}

} // namespace ir::tests

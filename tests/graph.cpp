#include "domTree/dfo_rpo.h"
#include "testBase.h"
#include <iostream>
#include <ranges>

namespace ir::tests {
class GraphTest : public TestBase {};

TEST_F(GraphTest, TestGraph1) {
    auto instType = InstType::i32;
    auto *mul = GetInstructionBuilder()->BuildMul(instType, nullptr, nullptr);

    // Add a single instruction in the 1st basic block
    auto *bb = GetGraph()->CreateEmptyBB();
    GetInstructionBuilder()->PushBackInst(bb, mul);
    ASSERT_EQ(mul->GetInstBB(), bb);
    ASSERT_EQ(bb->GetFirstInstBB(), mul);
    ASSERT_EQ(bb->GetLastInstBB(), mul);

    auto *addi1 = GetInstructionBuilder()->BuildAddi(instType, nullptr, 32);
    auto *addi2 = GetInstructionBuilder()->BuildAddi(instType, nullptr, 32);

    // Add another instruction in the 2nd basic block, which must be the
    // predecessor of the 1st
    auto *predBBlock1 = GetGraph()->CreateEmptyBB();
    GetInstructionBuilder()->PushBackInst(predBBlock1, addi1);
    bb->GetGraph()->AddBBBefore(bb, predBBlock1);
    ASSERT_EQ(predBBlock1->GetFirstInstBB(), addi1);
    ASSERT_EQ(predBBlock1->GetLastInstBB(), addi1);
    auto succs = predBBlock1->GetSuccessors();
    ASSERT_EQ(succs.size(), 1);
    ASSERT_EQ(succs[0], bb);
    auto preds = bb->GetPredecessors();
    ASSERT_EQ(preds.size(), 1);
    ASSERT_EQ(preds[0], predBBlock1);

    // Add instruction in the 3rd basic block, which must be between the 1st and
    // 2nd
    auto *predBBlock2 = GetGraph()->CreateEmptyBB();
    GetInstructionBuilder()->PushBackInst(predBBlock2, addi2);
    bb->GetGraph()->AddBBBefore(bb, predBBlock2);
    ASSERT_EQ(predBBlock2->GetFirstInstBB(), addi2);
    ASSERT_EQ(predBBlock2->GetLastInstBB(), addi2);
    succs = predBBlock1->GetSuccessors();
    ASSERT_EQ(succs.size(), 1);
    ASSERT_EQ(succs[0], predBBlock2);
    preds = predBBlock1->GetPredecessors();
    ASSERT_TRUE(preds.empty());
    preds = bb->GetPredecessors();
    ASSERT_EQ(preds.size(), 1);
    ASSERT_EQ(preds[0], predBBlock2);
    succs = predBBlock2->GetSuccessors();
    ASSERT_EQ(succs.size(), 1);
    ASSERT_EQ(succs[0], bb);
    GetGraph()->PrintSSA();
}

TEST_F(GraphTest, TestGraph2) {
    auto instType = InstType::i32;
    auto *mul = GetInstructionBuilder()->BuildMul(instType, nullptr, nullptr);
    auto *addi1 = GetInstructionBuilder()->BuildAddi(instType, nullptr, 32);
    auto *addi2 = GetInstructionBuilder()->BuildAddi(instType, nullptr, 32);

    // Create basic blocks as: [addi1] -> [addi2] -> [mul]
    auto *mulBBlock = GetGraph()->CreateEmptyBB();
    GetInstructionBuilder()->PushBackInst(mulBBlock, mul);

    auto *addiBBlock1 = GetGraph()->CreateEmptyBB();
    GetInstructionBuilder()->PushBackInst(addiBBlock1, addi1);
    GetGraph()->AddBBBefore(mulBBlock, addiBBlock1);

    auto *addiBBlock2 = GetGraph()->CreateEmptyBB();
    GetInstructionBuilder()->PushBackInst(addiBBlock2, addi2);
    GetGraph()->AddBBBefore(mulBBlock, addiBBlock2);

    GetGraph()->PrintSSA();

    // Unlink middle basic block and check results
    GetGraph()->SetBBAsDead(addiBBlock2);
    ASSERT_TRUE(addiBBlock2->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock2->GetPredecessors().empty());
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(mulBBlock->GetPredecessors().empty());

    // Unlink last basic block and check results
    GetGraph()->SetBBAsDead(mulBBlock);
    ASSERT_TRUE(mulBBlock->GetSuccessors().empty());
    ASSERT_TRUE(mulBBlock->GetPredecessors().empty());
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock1->GetPredecessors().empty());

    // Unlink the remaining basic block and check results
    GetGraph()->SetBBAsDead(addiBBlock1);
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock1->GetPredecessors().empty());
}

// creation of multiple linked basic blocks
TEST_F(GraphTest, TestLinkedBlocks) {
    // Create two basic blocks
    auto *block1 = GetGraph()->CreateEmptyBB();
    auto *block2 = GetGraph()->CreateEmptyBB();

    // Link the blocks
    block1->GetGraph()->ConnectBBs(block1, block2);
    ASSERT_EQ(block1->GetSuccessors().size(), 1);
    ASSERT_EQ(block1->GetSuccessors()[0], block2);
    ASSERT_EQ(block2->GetPredecessors().size(), 1);
    ASSERT_EQ(block2->GetPredecessors()[0], block1);
    GetGraph()->PrintSSA();
}

// graph unlinking edge cases (isolated block)
TEST_F(GraphTest, TestIsolatedBlockUnlink) {
    // Create an isolated basic block
    auto *block = GetGraph()->CreateEmptyBB();

    // Verify it has no predecessors or successors initially
    ASSERT_TRUE(block->GetPredecessors().empty());
    ASSERT_TRUE(block->GetSuccessors().empty());
    GetGraph()->PrintSSA();

    // Unlink the block (even though it's not connected) and verify it still has
    // no links
    GetGraph()->SetBBAsDead(block);
    ASSERT_TRUE(block->GetPredecessors().empty());
    ASSERT_TRUE(block->GetSuccessors().empty());
}

TEST_F(GraphTest, TestGraphCopy) {

    auto *graph = GetGraph();
    std::vector<BB *> bblocks(11);
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

    std::pair<Graph *, std::vector<BB *>> preBuiltGraph = {graph, bblocks};
    Graph *originalGraph = preBuiltGraph.first;
    Graph *copyGraph = compiler_.CopyGraph(
        originalGraph, originalGraph->GetInstructionBuilder());

    auto origRPO = RPO(originalGraph);
    auto copyRPO = RPO(copyGraph);

    ASSERT_EQ(origRPO.size(), copyRPO.size());
    for (int i = 0, bblockCount = origRPO.size(); i < bblockCount; ++i) {
        ASSERT_NE(copyRPO[i], nullptr);
        ASSERT_EQ(origRPO[i]->GetSize(), copyRPO[i]->GetSize());

        auto origInstr = origRPO[i]->begin();
        auto copyInstr = copyRPO[i]->begin();
        while (origInstr != origRPO[i]->end() &&
               copyInstr != copyRPO[i]->end()) {
            ASSERT_NE(*copyInstr, nullptr);
            ASSERT_EQ((*origInstr)->GetInstID(), (*copyInstr)->GetInstID());
            ASSERT_EQ((*origInstr)->GetOpcode(), (*copyInstr)->GetOpcode());

            ++origInstr;
            ++copyInstr;
        }
    }
}

} // namespace ir::tests

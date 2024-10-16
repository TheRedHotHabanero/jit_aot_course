#include "helperBuilderFunctions.h"
#include "irGen.h"
#include "gtest/gtest.h"
#include <iostream>

namespace ir::tests {
class GraphTest : public ::testing::Test {
  public:
    virtual void SetUp() { irGenerator.CreateGraph(); }

    GraphTest() = default;

    virtual void TearDown() {
        instrBuilder.Clear();
        irGenerator.Clear();
    }

    InstructionBuilder instrBuilder;
    IRGenerator irGenerator;
};

TEST_F(GraphTest, TestGraph1) {
    auto instType = InstType::i32;
    auto *mul = instrBuilder.BuildMul(instType, nullptr, nullptr);

    // Add a single instruction in the 1st basic block
    auto *bb = irGenerator.CreateEmptyBB();
    instrBuilder.PushBackInst(bb, mul);
    ASSERT_EQ(mul->GetInstBB(), bb);
    ASSERT_EQ(bb->GetFirstInstBB(), mul);
    ASSERT_EQ(bb->GetLastInstBB(), mul);

    auto *addi1 = instrBuilder.BuildAddi(instType, nullptr, 32);
    auto *addi2 = instrBuilder.BuildAddi(instType, nullptr, 32);

    // Add another instruction in the 2nd basic block, which must be the
    // predecessor of the 1st
    auto *predBBlock1 = irGenerator.CreateEmptyBB();
    instrBuilder.PushBackInst(predBBlock1, addi1);
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
    auto *predBBlock2 = irGenerator.CreateEmptyBB();
    instrBuilder.PushBackInst(predBBlock2, addi2);
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
    irGenerator.GetGraph()->PrintSSA();
}

TEST_F(GraphTest, TestGraph2) {
    auto instType = InstType::i32;
    auto *mul = instrBuilder.BuildMul(instType, nullptr, nullptr);
    auto *addi1 = instrBuilder.BuildAddi(instType, nullptr, 32);
    auto *addi2 = instrBuilder.BuildAddi(instType, nullptr, 32);

    // Create basic blocks as: [addi1] -> [addi2] -> [mul]
    auto *mulBBlock = irGenerator.CreateEmptyBB();
    instrBuilder.PushBackInst(mulBBlock, mul);

    auto *addiBBlock1 = irGenerator.CreateEmptyBB();
    instrBuilder.PushBackInst(addiBBlock1, addi1);
    irGenerator.GetGraph()->AddBBBefore(mulBBlock, addiBBlock1);

    auto *addiBBlock2 = irGenerator.CreateEmptyBB();
    instrBuilder.PushBackInst(addiBBlock2, addi2);
    irGenerator.GetGraph()->AddBBBefore(mulBBlock, addiBBlock2);

    irGenerator.GetGraph()->PrintSSA();

    // Unlink middle basic block and check results
    irGenerator.GetGraph()->SetBBAsDead(addiBBlock2);
    ASSERT_TRUE(addiBBlock2->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock2->GetPredecessors().empty());
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(mulBBlock->GetPredecessors().empty());

    // Unlink last basic block and check results
    irGenerator.GetGraph()->SetBBAsDead(mulBBlock);
    ASSERT_TRUE(mulBBlock->GetSuccessors().empty());
    ASSERT_TRUE(mulBBlock->GetPredecessors().empty());
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock1->GetPredecessors().empty());

    // Unlink the remaining basic block and check results
    irGenerator.GetGraph()->SetBBAsDead(addiBBlock1);
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock1->GetPredecessors().empty());
}

// creation of multiple linked basic blocks
TEST_F(GraphTest, TestLinkedBlocks) {
    // Create two basic blocks
    auto *block1 = irGenerator.CreateEmptyBB();
    auto *block2 = irGenerator.CreateEmptyBB();

    // Link the blocks
    block1->GetGraph()->ConnectBBs(block1, block2);
    ASSERT_EQ(block1->GetSuccessors().size(), 1);
    ASSERT_EQ(block1->GetSuccessors()[0], block2);
    ASSERT_EQ(block2->GetPredecessors().size(), 1);
    ASSERT_EQ(block2->GetPredecessors()[0], block1);
    irGenerator.GetGraph()->PrintSSA();
}

// graph unlinking edge cases (isolated block)
TEST_F(GraphTest, TestIsolatedBlockUnlink) {
    // Create an isolated basic block
    auto *block = irGenerator.CreateEmptyBB();

    // Verify it has no predecessors or successors initially
    ASSERT_TRUE(block->GetPredecessors().empty());
    ASSERT_TRUE(block->GetSuccessors().empty());
    irGenerator.GetGraph()->PrintSSA();

    // Unlink the block (even though it's not connected) and verify it still has
    // no links
    irGenerator.GetGraph()->SetBBAsDead(block);
    ASSERT_TRUE(block->GetPredecessors().empty());
    ASSERT_TRUE(block->GetSuccessors().empty());
}

} // namespace ir::tests

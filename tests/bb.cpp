#include "helperBuilderFunctions.h"
#include "irGen.h"
#include "gtest/gtest.h"
#include <iostream>

namespace ir::tests {
class BBTest : public ::testing::Test {
  public:
    virtual void SetUp() { irGenerator.CreateGraph(); }

    BBTest() = default;

    virtual void TearDown() {
        instructionBuilder.Clear();
        irGenerator.Clear();
    }

    InstructionBuilder instructionBuilder;
    IRGenerator irGenerator;
};

TEST_F(BBTest, TestBB1) {
    auto *bb = irGenerator.CreateEmptyBB();
    ASSERT_NE(bb, nullptr);
    ASSERT_EQ(bb->GetFirstInstBB(), nullptr);
    ASSERT_EQ(bb->GetLastInstBB(), nullptr);
    ASSERT_EQ(bb->GetGraph(), irGenerator.GetGraph());

    auto instType = InstType::i32;
    auto *mul = instructionBuilder.BuildMul(instType, nullptr, nullptr);

    // Add 1st instruction
    bb->PushInstBackward(mul);
    ASSERT_EQ(mul->GetInstBB(), bb);
    ASSERT_EQ(bb->GetFirstInstBB(), mul);
    ASSERT_EQ(bb->GetLastInstBB(), mul);

    auto *addi1 = instructionBuilder.BuildAddi(instType, nullptr, 32);
    auto *addi2 = instructionBuilder.BuildAddi(instType, nullptr, 32);

    // Add 2nd instruction to the start of the basic block
    bb->PushInstForward(addi2);
    ASSERT_EQ(addi2->GetInstBB(), bb);
    ASSERT_EQ(bb->GetFirstInstBB(), addi2);
    ASSERT_EQ(bb->GetLastInstBB(), mul);

    // Add 3rd instruction to the start of the basic block
    bb->PushInstForward(addi1);
    ASSERT_EQ(addi1->GetInstBB(), bb);
    ASSERT_EQ(bb->GetFirstInstBB(), addi1);
    ASSERT_EQ(bb->GetLastInstBB(), mul);
    ASSERT_EQ(addi1->GetNextInst(), addi2);
}

TEST_F(BBTest, TestBB2) {
    auto *bb = irGenerator.CreateEmptyBB();

    auto instType = InstType::i32;
    auto *mul = instructionBuilder.BuildMul(instType, nullptr, nullptr);
    auto *addi1 = instructionBuilder.BuildAddi(instType, nullptr, 32);
    auto *addi2 = instructionBuilder.BuildAddi(instType, nullptr, 32);

    // Add each instruction separately
    instructionBuilder.PushBackInst(bb, addi1);
    instructionBuilder.PushBackInst(bb, addi2);
    instructionBuilder.PushBackInst(bb, mul);

    // Check correct order
    ASSERT_EQ(bb->GetFirstInstBB(), addi1);
    ASSERT_EQ(addi1->GetNextInst(), addi2);
    ASSERT_EQ(addi2->GetNextInst(), mul);
    ASSERT_EQ(bb->GetLastInstBB(), mul);
}

TEST_F(BBTest, TestBB3) {
    auto instType = InstType::i32;
    auto *mul = instructionBuilder.BuildMul(instType, nullptr, nullptr);
    auto *addi1 = instructionBuilder.BuildAddi(instType, nullptr, 32);
    auto *addi2 = instructionBuilder.BuildAddi(instType, nullptr, 32);

    // Add each instruction separately
    auto *bb = irGenerator.CreateEmptyBB();
    instructionBuilder.PushBackInst(bb, addi1);
    instructionBuilder.PushBackInst(bb, addi2);
    instructionBuilder.PushBackInst(bb, mul);

    // Unlink the 2nd instruction
    bb->SetInstructionAsDead(addi2);
    ASSERT_EQ(addi2->GetInstBB(), nullptr);
    ASSERT_EQ(addi2->GetPrevInst(), nullptr);
    ASSERT_EQ(addi2->GetNextInst(), nullptr);
    ASSERT_EQ(bb->GetFirstInstBB(), addi1);
    ASSERT_EQ(addi1->GetNextInst(), mul);
    ASSERT_EQ(bb->GetLastInstBB(), mul);
    ASSERT_EQ(mul->GetPrevInst(), addi1);

    // Unlink the 3rd instruction
    bb->SetInstructionAsDead(mul);
    ASSERT_EQ(mul->GetInstBB(), nullptr);
    ASSERT_EQ(mul->GetPrevInst(), nullptr);
    ASSERT_EQ(mul->GetNextInst(), nullptr);
    ASSERT_EQ(bb->GetFirstInstBB(), addi1);
    ASSERT_EQ(bb->GetLastInstBB(), addi1);
    ASSERT_EQ(addi1->GetPrevInst(), nullptr);
    ASSERT_EQ(addi1->GetNextInst(), nullptr);
}

TEST_F(BBTest, TestBBCreateCmp) {
    auto instType = InstType::i32;
    auto *cmp =
        instructionBuilder.BuildCmp(instType, Conditions::EQ, nullptr, nullptr);

    auto *bb = irGenerator.CreateEmptyBB();
    bb->PushInstBackward(cmp);
    ASSERT_EQ(bb->GetFirstInstBB(), cmp);
    ASSERT_EQ(bb->GetLastInstBB(), cmp);
}

TEST_F(BBTest, TestBBCreateCast) {
    auto fromType = InstType::i32;
    auto toType = InstType::i64;
    auto *cast = instructionBuilder.BuildCast(fromType, toType, nullptr);
    auto *bb = irGenerator.CreateEmptyBB();
    bb->PushInstBackward(cast);
    ASSERT_EQ(bb->GetFirstInstBB(), cast);
    ASSERT_EQ(bb->GetLastInstBB(), cast);
}

TEST_F(BBTest, TestBBCreatePhi) {
    auto instType = InstType::i32;
    auto *phi = instructionBuilder.BuildPhi(instType);

    auto *bb = irGenerator.CreateEmptyBB();
    bb->PushInstBackward(phi);
    ASSERT_EQ(bb->GetFirstInstBB(), phi);
    ASSERT_EQ(bb->GetLastInstBB(), phi);
}
} // namespace ir::tests

#include "testBase.h"
#include <iostream>

namespace ir::tests {
class BBTest : public TestBase {
};

TEST_F(BBTest, TestBB1) {
    auto *bb = GetIRGenerator().CreateEmptyBB();
    ASSERT_NE(bb, nullptr);
    ASSERT_EQ(bb->GetFirstInstBB(), nullptr);
    ASSERT_EQ(bb->GetLastInstBB(), nullptr);
    ASSERT_EQ(bb->GetGraph(), GetIRGenerator().GetGraph());

    auto instType = InstType::i32;
    auto *mul = GetInstructionBuilder().BuildMul(instType, nullptr, nullptr);

    // Add 1st instruction
    bb->PushInstBackward(mul);
    ASSERT_EQ(mul->GetInstBB(), bb);
    ASSERT_EQ(bb->GetFirstInstBB(), mul);
    ASSERT_EQ(bb->GetLastInstBB(), mul);
    auto imm = 32UL;
    auto *addi1 = GetInstructionBuilder().BuildAddi(instType, nullptr, imm);
    auto *addi2 = GetInstructionBuilder().BuildAddi(instType, nullptr, imm);

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
    auto *bb = GetIRGenerator().CreateEmptyBB();

    auto instType = InstType::i32;
    auto *mul = GetInstructionBuilder().BuildMul(instType, nullptr, nullptr);
    auto imm = 32UL;
    auto *addi1 = GetInstructionBuilder().BuildAddi(instType, nullptr, imm);
    auto *addi2 = GetInstructionBuilder().BuildAddi(instType, nullptr, imm);

    // Add each instruction separately
    GetInstructionBuilder().PushBackInst(bb, addi1);
    GetInstructionBuilder().PushBackInst(bb, addi2);
    GetInstructionBuilder().PushBackInst(bb, mul);

    // Check correct order
    ASSERT_EQ(bb->GetFirstInstBB(), addi1);
    ASSERT_EQ(addi1->GetNextInst(), addi2);
    ASSERT_EQ(addi2->GetNextInst(), mul);
    ASSERT_EQ(bb->GetLastInstBB(), mul);
}

TEST_F(BBTest, TestBB3) {
    auto instType = InstType::i32;
    auto *mul = GetInstructionBuilder().BuildMul(instType, nullptr, nullptr);
    auto imm = 32UL;
    auto *addi1 = GetInstructionBuilder().BuildAddi(instType, nullptr, imm);
    auto *addi2 = GetInstructionBuilder().BuildAddi(instType, nullptr, imm);

    // Add each instruction separately
    auto *bb = GetIRGenerator().CreateEmptyBB();
    GetInstructionBuilder().PushBackInst(bb, addi1);
    GetInstructionBuilder().PushBackInst(bb, addi2);
    GetInstructionBuilder().PushBackInst(bb, mul);

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
        GetInstructionBuilder().BuildCmp(instType, Conditions::EQ, nullptr, nullptr);

    auto *bb = GetIRGenerator().CreateEmptyBB();
    bb->PushInstBackward(cmp);
    ASSERT_EQ(bb->GetFirstInstBB(), cmp);
    ASSERT_EQ(bb->GetLastInstBB(), cmp);
}

TEST_F(BBTest, TestBBCreateCast) {
    auto fromType = InstType::i32;
    auto toType = InstType::i64;
    auto *cast = GetInstructionBuilder().BuildCast(fromType, toType, nullptr);
    auto *bb = GetIRGenerator().CreateEmptyBB();
    bb->PushInstBackward(cast);
    ASSERT_EQ(bb->GetFirstInstBB(), cast);
    ASSERT_EQ(bb->GetLastInstBB(), cast);
}
} // namespace ir::tests

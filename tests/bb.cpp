#include "testBase.h"
#include <iostream>

namespace ir::tests {
class BBTest : public TestBase {};

TEST_F(BBTest, TestBB1) {
    auto *graph = GetGraph();
    auto *bb = graph->CreateEmptyBB();
    ASSERT_NE(bb, nullptr);
    ASSERT_EQ(bb->GetFirstInstBB(), nullptr);
    ASSERT_EQ(bb->GetLastInstBB(), nullptr);
    ASSERT_EQ(bb->GetGraph(), GetGraph());

    auto instType = InstType::i32;
    auto *mul = GetInstructionBuilder()->BuildMul(instType, nullptr, nullptr);

    // Add 1st instruction
    bb->PushInstBackward(mul);
    ASSERT_EQ(mul->GetInstBB(), bb);
    ASSERT_EQ(bb->GetFirstInstBB(), mul);
    ASSERT_EQ(bb->GetLastInstBB(), mul);
    auto imm = 32UL;
    auto *addi1 = GetInstructionBuilder()->BuildAddi(instType, nullptr, imm);
    auto *addi2 = GetInstructionBuilder()->BuildAddi(instType, nullptr, imm);

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
    auto *bb = GetGraph()->CreateEmptyBB();

    auto instType = InstType::i32;
    auto *mul = GetInstructionBuilder()->BuildMul(instType, nullptr, nullptr);
    auto imm = 32UL;
    auto *addi1 = GetInstructionBuilder()->BuildAddi(instType, nullptr, imm);
    auto *addi2 = GetInstructionBuilder()->BuildAddi(instType, nullptr, imm);

    // Add each instruction separately
    GetInstructionBuilder()->PushBackInst(bb, addi1);
    GetInstructionBuilder()->PushBackInst(bb, addi2);
    GetInstructionBuilder()->PushBackInst(bb, mul);

    // Check correct order
    ASSERT_EQ(bb->GetFirstInstBB(), addi1);
    ASSERT_EQ(addi1->GetNextInst(), addi2);
    ASSERT_EQ(addi2->GetNextInst(), mul);
    ASSERT_EQ(bb->GetLastInstBB(), mul);
}

TEST_F(BBTest, TestBB3) {
    auto instType = InstType::i32;
    auto *mul = GetInstructionBuilder()->BuildMul(instType, nullptr, nullptr);
    auto imm = 32UL;
    auto *addi1 = GetInstructionBuilder()->BuildAddi(instType, nullptr, imm);
    auto *addi2 = GetInstructionBuilder()->BuildAddi(instType, nullptr, imm);

    // Add each instruction separately
    auto *bb = GetGraph()->CreateEmptyBB();
    GetInstructionBuilder()->PushBackInst(bb, addi1);
    GetInstructionBuilder()->PushBackInst(bb, addi2);
    GetInstructionBuilder()->PushBackInst(bb, mul);

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

TEST_F(BBTest, TestBBBuildCmp) {
    auto instType = InstType::i32;
    auto *cmp = GetInstructionBuilder()->BuildCmp(instType, Conditions::EQ,
                                                  nullptr, nullptr);

    auto *bb = GetGraph()->CreateEmptyBB();
    bb->PushInstBackward(cmp);
    ASSERT_EQ(bb->GetFirstInstBB(), cmp);
    ASSERT_EQ(bb->GetLastInstBB(), cmp);
}

TEST_F(BBTest, TestBBCreateCast) {
    auto fromType = InstType::i32;
    auto toType = InstType::i64;
    auto *cast = GetInstructionBuilder()->BuildCast(fromType, toType, nullptr);
    auto *bb = GetGraph()->CreateEmptyBB();
    bb->PushInstBackward(cast);
    ASSERT_EQ(bb->GetFirstInstBB(), cast);
    ASSERT_EQ(bb->GetLastInstBB(), cast);
}

TEST_F(BBTest, TestSplitAfterInstruction) {
    auto *instrBuilder = GetInstructionBuilder();
    auto type = InstType::i16;
    auto *graph = GetGraph();

    auto *splittedBlock = graph->CreateEmptyBB();
    graph->SetFirstBB(splittedBlock);
    auto *arg0 = instrBuilder->BuildArg(type);
    auto *arg1 = instrBuilder->BuildArg(type);
    memory::ArenaVector<Input> args(GetGraph()->GetAllocator()->ToSTL());
    args.push_back(arg0);
    args.push_back(arg1);
    auto *call = instrBuilder->BuildCall(type, ir::INVALID_BB_ID, args);
    auto *addi = instrBuilder->BuildAddi(type, call, 3);
    auto *constTen = instrBuilder->BuildConst(type, 10);
    auto *cmp =
        instrBuilder->BuildCmp(type, Conditions::GRTHAN, addi, constTen);
    auto *jcmp = instrBuilder->BuildJcmp();
    instrBuilder->PushBackInst(splittedBlock, arg0);
    instrBuilder->PushBackInst(splittedBlock, arg1);
    instrBuilder->PushBackInst(splittedBlock, call);
    instrBuilder->PushBackInst(splittedBlock, addi);
    instrBuilder->PushBackInst(splittedBlock, constTen);
    instrBuilder->PushBackInst(splittedBlock, cmp);
    instrBuilder->PushBackInst(splittedBlock, jcmp);

    auto *trueBranch = graph->CreateEmptyBB(true);
    graph->ConnectBBs(splittedBlock, trueBranch);
    auto *muli = instrBuilder->BuildMuli(type, call, 2);
    auto *ret1 = instrBuilder->BuildRet(type, muli);
    instrBuilder->PushBackInst(trueBranch, muli);
    instrBuilder->PushBackInst(trueBranch, ret1);

    auto *falseBranch = graph->CreateEmptyBB(true);
    graph->ConnectBBs(splittedBlock, falseBranch);
    auto *ret2 = instrBuilder->BuildRet(type, addi);
    instrBuilder->PushBackInst(falseBranch, ret2);
    ASSERT_EQ(graph->GetBBCount(), 4);

    auto splitted = call->GetInstBB()->SplitAfterInstruction(call, false);

    ASSERT_EQ(graph->GetBBCount(), 5);
    ASSERT_EQ(call->GetNextInst(), nullptr);
    ASSERT_TRUE(splittedBlock->GetSuccessors().empty());
    ASSERT_EQ(splittedBlock->GetSize(), 3);
    ASSERT_EQ(splittedBlock->GetFirstInstBB(), arg0);
    ASSERT_EQ(splittedBlock->GetLastInstBB(), call);
    auto *newBlock = splitted.second;
    ASSERT_TRUE(newBlock->GetPredecessors().empty());
    ASSERT_EQ(newBlock->GetSize(), 4);
    ASSERT_EQ(newBlock->GetFirstInstBB(), addi);
    ASSERT_EQ(newBlock->GetFirstInstBB()->GetPrevInst(), nullptr);
    ASSERT_EQ(newBlock->GetLastInstBB(), jcmp);
}

} // namespace ir::tests

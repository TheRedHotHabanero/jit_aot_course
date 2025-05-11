#include "testBase.h"
#include <iostream>

namespace ir::tests {
class InstructionsTest : public TestBase {};

TEST_F(InstructionsTest, TestMul) {
    auto instType = InstType::i32;
    auto *arg1 = GetInstructionBuilder()->BuildArg(instType);
    auto *arg2 = GetInstructionBuilder()->BuildArg(instType);
    auto *instr = GetInstructionBuilder()->BuildMul(instType, arg1, arg2);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::MUL);
    ASSERT_EQ(instr->GetType(), instType);
    // ASSERT_EQ(instr->GetInput(0), arg1);
    // ASSERT_EQ(instr->GetInput(1), arg2);
}

TEST_F(InstructionsTest, TestAddi) {
    auto instType = InstType::u64;
    auto *arg = GetInstructionBuilder()->BuildArg(instType);
    auto imm = 11UL;
    auto *instr = GetInstructionBuilder()->BuildAddi(instType, arg, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::ADDI);
    ASSERT_EQ(instr->GetType(), instType);
    // ASSERT_EQ(instr->GetInput(), arg);
    // ASSERT_EQ(instr->GetValue(), imm);
}

TEST_F(InstructionsTest, TestConst) {
    auto instType = InstType::i64;
    auto imm = 12L;
    auto *instr = GetInstructionBuilder()->BuildConst(instType, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CONST);
    ASSERT_EQ(instr->GetType(), instType);
    ASSERT_EQ(instr->GetValue(), imm);
}

TEST_F(InstructionsTest, TestCast) {
    auto fromType = InstType::i32;
    auto toType = InstType::u8;
    auto *arg = GetInstructionBuilder()->BuildArg(fromType);
    auto *instr = GetInstructionBuilder()->BuildCast(fromType, toType, arg);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CAST);
    ASSERT_EQ(instr->GetType(), fromType);
    ASSERT_EQ(instr->GetTargetType(), toType);
    // ASSERT_EQ(instr->GetInput(), arg);
}

TEST_F(InstructionsTest, TestCmp) {
    auto instType = InstType::u64;
    auto conditions = Conditions::LSTHAN;
    auto *arg1 = GetInstructionBuilder()->BuildArg(instType);
    auto *arg2 = GetInstructionBuilder()->BuildArg(instType);
    auto *instr =
        GetInstructionBuilder()->BuildCmp(instType, conditions, arg1, arg2);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CMP);
    ASSERT_EQ(instr->GetInput(0), arg1);
    ASSERT_EQ(instr->GetInput(1), arg2);
    // ASSERT_EQ(instr->GetVReg1(), vreg1);
    // ASSERT_EQ(instr->GetVReg2(), vreg2);
}

TEST_F(InstructionsTest, TestJumpCMP) {
    auto *graph = GetGraph();
    auto *instrBuilder = GetInstructionBuilder();
    auto opType = InstType::i16;

    auto *bblockSource = graph->CreateEmptyBB();
    graph->SetFirstBB(bblockSource);
    auto *lhs = instrBuilder->BuildArg(opType);
    auto *rhs = instrBuilder->BuildArg(opType);
    auto *cmp = instrBuilder->BuildCmp(opType, Conditions::GRTHAN, lhs, rhs);
    auto *jcmp = instrBuilder->BuildJcmp();
    instrBuilder->PushBackInst(bblockSource, lhs);
    instrBuilder->PushBackInst(bblockSource, rhs);
    instrBuilder->PushBackInst(bblockSource, cmp);
    instrBuilder->PushBackInst(bblockSource, jcmp);

    auto *bblockTrue = graph->CreateEmptyBB();
    graph->ConnectBBs(bblockSource, bblockTrue);
    auto *bblockFalse = graph->CreateEmptyBB();
    graph->ConnectBBs(bblockSource, bblockFalse);

    ASSERT_NE(jcmp, nullptr);
    ASSERT_EQ(jcmp->GetOpcode(), Opcode::JCMP);
    ASSERT_EQ(jcmp->GetType(), InstType::i64);
    ASSERT_EQ(jcmp->GetTrueDestination(), bblockTrue);
    ASSERT_EQ(jcmp->GetFalseDestination(), bblockFalse);
}

TEST_F(InstructionsTest, TestJmp) {
    auto *graph = GetGraph();
    auto *instrBuilder = GetInstructionBuilder();

    auto *bblockSource = graph->CreateEmptyBB();
    graph->SetFirstBB(bblockSource);
    auto *jmp = instrBuilder->BuildJmp();
    instrBuilder->PushBackInst(bblockSource, jmp);

    auto *bblockDest = graph->CreateEmptyBB();
    graph->ConnectBBs(bblockSource, bblockDest);

    ASSERT_NE(jmp, nullptr);
    ASSERT_EQ(jmp->GetOpcode(), Opcode::JMP);
    ASSERT_EQ(jmp->GetType(), InstType::i64);
    ASSERT_EQ(jmp->GetDestination(), bblockDest);
}

TEST_F(InstructionsTest, TestCall) {
    auto *instrBuilder = GetInstructionBuilder();

    auto opType = InstType::u8;
    FunctionID callTarget = 1;

    auto *call = instrBuilder->BuildCall(opType, callTarget);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->GetOpcode(), Opcode::CALL);
    ASSERT_EQ(call->GetType(), opType);
    ASSERT_EQ(call->GetCallTarget(), callTarget);
    ASSERT_EQ(call->GetInputsCount(), 0);

    auto *arg0 = instrBuilder->BuildArg(InstType::u16);
    auto *arg1 = instrBuilder->BuildArg(InstType::i64);
    callTarget = 7;
    call = instrBuilder->BuildCall(opType, callTarget, {arg0, arg1});
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->GetOpcode(), Opcode::CALL);
    ASSERT_EQ(call->GetType(), opType);
    ASSERT_EQ(call->GetCallTarget(), callTarget);
    ASSERT_EQ(call->GetInputsCount(), 2);
    ASSERT_EQ(call->GetInput(0), arg0);
    ASSERT_EQ(call->GetInput(1), arg1);
}

TEST_F(InstructionsTest, TestLoad) {
    auto opType = InstType::u64;
    uint64_t addr = 44;

    auto *instr = GetInstructionBuilder()->BuildLoad(opType, addr);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::LOAD);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetValue(), addr);
}

TEST_F(InstructionsTest, TestStore) {
    auto opType = InstType::u32;
    auto *storedValue = GetInstructionBuilder()->BuildArg(opType);
    uint64_t addr = 34;

    auto *instr = GetInstructionBuilder()->BuildStore(storedValue, addr);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::STORE);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetValue(), addr);
    ASSERT_EQ(instr->GetInputsCount(), 1);
    ASSERT_EQ(instr->GetInput(0), storedValue);
}

TEST_F(InstructionsTest, TestRet) {
    auto instType = InstType::u8;
    auto *arg = GetInstructionBuilder()->BuildArg(instType);
    auto *instr = GetInstructionBuilder()->BuildRet(instType, arg);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::RET);
    ASSERT_EQ(instr->GetType(), instType);
    // ASSERT_EQ(instr->GetInput(), arg);
}

} // namespace ir::tests

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
    ASSERT_EQ(instr->GetInput(0), arg1);
    ASSERT_EQ(instr->GetInput(1), arg2);
}

TEST_F(InstructionsTest, TestAddi) {
    auto instType = InstType::u64;
    auto *arg = GetInstructionBuilder()->BuildArg(instType);
    auto imm = 11UL;
    auto *instr = GetInstructionBuilder()->BuildAddi(instType, arg, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::ADDI);
    ASSERT_EQ(instr->GetType(), instType);
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
    ASSERT_EQ(instr->GetInput(), arg);
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

TEST_F(InstructionsTest, TestNewArray) {
    auto *len = GetInstructionBuilder()->BuildConst(InstType::u64, 2);
    uint64_t typeId = 1234;
    auto *instr = GetInstructionBuilder()->BuildNewArray(len, typeId);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::NEW_ARRAY);
    ASSERT_EQ(instr->GetType(), InstType::REF);
    ASSERT_EQ(instr->GetTypeId(), typeId);
    ASSERT_EQ(instr->GetInputsCount(), 1);
    ASSERT_EQ(instr->GetInput(0), len);
}

TEST_F(InstructionsTest, TestNewArrayImm) {
    uint64_t len = 2;
    auto *instr = GetInstructionBuilder()->BuildNewArrayImm(len, 42);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::NEW_ARRAY_IMM);
    ASSERT_EQ(instr->GetType(), InstType::REF);
    ASSERT_EQ(instr->GetValue(), len);
    ASSERT_EQ(instr->GetTypeId(), 42);
}

TEST_F(InstructionsTest, TestLen) {
    auto *array = GetInstructionBuilder()->BuildNewArrayImm(2, 1234);
    auto *instr = GetInstructionBuilder()->BuildLen(array);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::LEN);
    ASSERT_EQ(instr->GetType(), InstType::u64);
    ASSERT_EQ(instr->GetInput(0), array);
}

TEST_F(InstructionsTest, TestNewObject) {
    uint64_t typeId = 1234;
    auto *instr = GetInstructionBuilder()->BuildNewObject(typeId);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::NEW_OBJECT);
    ASSERT_EQ(instr->GetType(), InstType::REF);
    ASSERT_EQ(instr->GetTypeId(), typeId);
}

TEST_F(InstructionsTest, TestLoadArray) {
    auto opType = InstType::u8;

    auto *constOne = GetInstructionBuilder()->BuildConst(InstType::u64, 1);
    auto *array = GetInstructionBuilder()->BuildNewArrayImm(2, 1234);
    auto *instr =
        GetInstructionBuilder()->BuildLoadArray(opType, array, constOne);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::LOAD_ARRAY);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetInputsCount(), 2);
    ASSERT_EQ(instr->GetInput(0), array);
    ASSERT_EQ(instr->GetInput(1), constOne);
}

TEST_F(InstructionsTest, TestLoadArrayImm) {
    auto opType = InstType::u8;

    uint64_t idx = 1;
    auto *array = GetInstructionBuilder()->BuildNewArrayImm(2, 1234);
    auto *instr =
        GetInstructionBuilder()->BuildLoadArrayImm(opType, array, idx);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::LOAD_ARRAY_IMM);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetInputsCount(), 1);
    ASSERT_EQ(instr->GetInput(0), array);
    ASSERT_EQ(instr->GetValue(), idx);
}

TEST_F(InstructionsTest, TestLoadObject) {
    auto opType = InstType::u8;

    uint64_t offset = 8;
    auto *obj = GetInstructionBuilder()->BuildNewObject(1234);
    auto *instr = GetInstructionBuilder()->BuildLoadObject(opType, obj, offset);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::LOAD_OBJECT);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetInputsCount(), 1);
    ASSERT_EQ(instr->GetInput(0), obj);
    ASSERT_EQ(instr->GetValue(), offset);
}

TEST_F(InstructionsTest, TestStoreArray) {
    auto *constOne = GetInstructionBuilder()->BuildConst(InstType::u64, 1);
    auto *value = GetInstructionBuilder()->BuildConst(InstType::u64, -1);
    auto *array = GetInstructionBuilder()->BuildNewArrayImm(2, 1234);
    auto *instr =
        GetInstructionBuilder()->BuildStoreArray(array, value, constOne);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::STORE_ARRAY);
    ASSERT_EQ(instr->GetInputsCount(), 3);
    ASSERT_EQ(instr->GetInput(0), array);
    ASSERT_EQ(instr->GetInput(1), value);
    ASSERT_EQ(instr->GetInput(2), constOne);
}

TEST_F(InstructionsTest, TestStoreArrayImm) {
    uint64_t idx = 1;
    auto *value = GetInstructionBuilder()->BuildConst(InstType::u64, -1);
    auto *array = GetInstructionBuilder()->BuildNewArrayImm(2, 1234);
    auto *instr =
        GetInstructionBuilder()->BuildStoreArrayImm(array, value, idx);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::STORE_ARRAY_IMM);
    ASSERT_EQ(instr->GetInputsCount(), 2);
    ASSERT_EQ(instr->GetInput(0), array);
    ASSERT_EQ(instr->GetInput(1), value);
    ASSERT_EQ(instr->GetValue(), idx);
}

TEST_F(InstructionsTest, TestStoreObject) {
    uint64_t offset = 8;
    auto *value = GetInstructionBuilder()->BuildConst(InstType::u64, -1);
    auto *obj = GetInstructionBuilder()->BuildNewObject(1234);
    auto *instr = GetInstructionBuilder()->BuildStoreObject(obj, value, offset);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::STORE_OBJECT);
    ASSERT_EQ(instr->GetInputsCount(), 2);
    ASSERT_EQ(instr->GetInput(0), obj);
    ASSERT_EQ(instr->GetInput(1), value);
    ASSERT_EQ(instr->GetValue(), offset);
}

TEST_F(InstructionsTest, TestNullCheck) {
    auto *obj = GetInstructionBuilder()->BuildNewObject(42);
    auto *instr = GetInstructionBuilder()->BuildNullCheck(obj);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::NULL_CHECK);
    ASSERT_EQ(instr->GetInputsCount(), 1);
    ASSERT_EQ(instr->GetInput(0), obj);
}

TEST_F(InstructionsTest, TestBoundsCheck) {
    auto *arr = GetInstructionBuilder()->BuildNewArrayImm(42, 42);
    auto *idx = GetInstructionBuilder()->BuildConst(InstType::i16, 1);
    auto *instr = GetInstructionBuilder()->BuildBoundsCheck(arr, idx);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::BOUNDS_CHECK);
    ASSERT_EQ(instr->GetInputsCount(), 2);
    ASSERT_EQ(instr->GetInput(0), arr);
    ASSERT_EQ(instr->GetInput(1), idx);
}

} // namespace ir::tests

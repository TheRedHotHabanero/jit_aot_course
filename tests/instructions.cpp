#include "testBase.h"
#include <iostream>

namespace ir::tests {
class InstructionsTest : public TestBase {
};

TEST_F(InstructionsTest, TestMul) {
    auto instType = InstType::i32;
    auto *arg1 = GetInstructionBuilder().BuildArg(instType);
    auto *arg2 = GetInstructionBuilder().BuildArg(instType);
    auto *instr = GetInstructionBuilder().BuildMul(instType, arg1, arg2);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::MUL);
    ASSERT_EQ(instr->GetType(), instType);
    // ASSERT_EQ(instr->GetInput(0), arg1);
    // ASSERT_EQ(instr->GetInput(1), arg2);
}

TEST_F(InstructionsTest, TestAddi) {
    auto instType = InstType::u64;
    auto *arg = GetInstructionBuilder().BuildArg(instType);
    auto imm = 11UL;
    auto *instr = GetInstructionBuilder().BuildAddi(instType, arg, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::ADDI);
    ASSERT_EQ(instr->GetType(), instType);
    // ASSERT_EQ(instr->GetInput(), arg);
    // ASSERT_EQ(instr->GetValue(), imm);
}

TEST_F(InstructionsTest, TestConst) {
    auto instType = InstType::i64;
    auto imm = 12L;
    auto *instr = GetInstructionBuilder().BuildConst(instType, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CONST);
    ASSERT_EQ(instr->GetType(), instType);
    ASSERT_EQ(instr->GetValue(), imm);
}

TEST_F(InstructionsTest, TestCast) {
    auto fromType = InstType::i32;
    auto toType = InstType::u8;
    auto *arg = GetInstructionBuilder().BuildArg(fromType);
    auto *instr = GetInstructionBuilder().BuildCast(fromType, toType, arg);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CAST);
    ASSERT_EQ(instr->GetType(), fromType);
    ASSERT_EQ(instr->GetTargetType(), toType);
    // ASSERT_EQ(instr->GetInput(), arg);
}

TEST_F(InstructionsTest, TestCmp) {
    auto instType = InstType::u64;
    auto conditions = Conditions::LSTHAN;
    auto *arg1 = GetInstructionBuilder().BuildArg(instType);
    auto *arg2 = GetInstructionBuilder().BuildArg(instType);
    auto *instr = GetInstructionBuilder().BuildCmp(instType, conditions, arg1, arg2);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CMP);
    ASSERT_EQ(instr->GetInput(0), arg1);
    ASSERT_EQ(instr->GetInput(1), arg2);
    // ASSERT_EQ(instr->GetVReg1(), vreg1);
    // ASSERT_EQ(instr->GetVReg2(), vreg2);
}

TEST_F(InstructionsTest, TestJa) {
    auto dest = 42;
    auto *instr = GetInstructionBuilder().BuildJa(dest);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::JA);
    ASSERT_EQ(instr->GetType(), InstType::i64);
    ASSERT_EQ(instr->GetValue(), dest);
}

TEST_F(InstructionsTest, TestJmp) {
    int64_t dest = 42;
    auto *instr = GetInstructionBuilder().BuildJmp(dest);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::JMP);
    ASSERT_EQ(instr->GetType(), InstType::i64);
    ASSERT_EQ(instr->GetValue(), dest);
}

TEST_F(InstructionsTest, TestRet) {
    auto instType = InstType::u8;
    auto *arg = GetInstructionBuilder().BuildArg(instType);
    auto *instr = GetInstructionBuilder().BuildRet(instType, arg);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::RET);
    ASSERT_EQ(instr->GetType(), instType);
    // ASSERT_EQ(instr->GetInput(), arg);
}

} // namespace ir::tests

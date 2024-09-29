#include "helperBuilderFunctions.h"
#include "irGen.h"
#include "gtest/gtest.h"

namespace ir::tests {
class InstructionsTest : public ::testing::Test {
  public:
    InstructionsTest() = default;

    virtual void TearDown() {
        instructionBuilder.Clear();
        irGenerator.Clear();
    }

    InstructionBuilder instructionBuilder;
    IRGenerator irGenerator;
};

TEST_F(InstructionsTest, TestMul) {
    auto instType = InstType::i32;
    auto vdest = VReg(0);
    auto vreg1 = VReg(1);
    auto vreg2 = VReg(2);
    auto *instr = instructionBuilder.BuildMul(instType, vdest, vreg1, vreg2);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::MUL);
    ASSERT_EQ(instr->GetRegType(), instType);
    // ASSERT_EQ(instr->GetDestVReg(), vdest);
    // ASSERT_EQ(instr->GetVReg1(), vreg1);
    // ASSERT_EQ(instr->GetVReg2(), vreg2);
}

TEST_F(InstructionsTest, TestAddi) {
    auto instType = InstType::u64;
    auto vdest = VReg(0);
    auto vreg = VReg(1);
    auto imm = 11UL;
    auto *instr = instructionBuilder.BuildAddi(instType, vdest, vreg, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::ADDI);
    ASSERT_EQ(instr->GetRegType(), instType);
    // ASSERT_EQ(instr->GetDestVReg(), vdest);
    // ASSERT_EQ(instr->GetVReg(), vreg);
    ASSERT_EQ(instr->GetImm(), imm);
}

TEST_F(InstructionsTest, TestMovi) {
    auto instType = InstType::i64;
    auto vdest = VReg(0);
    auto imm = 12L;
    auto *instr = instructionBuilder.BuildMovi(instType, vdest, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::MOVI);
    ASSERT_EQ(instr->GetRegType(), instType);
    // ASSERT_EQ(instr->GetDestVReg(), vdest);
    ASSERT_EQ(instr->GetImm(), imm);
}

TEST_F(InstructionsTest, TestCast) {
    auto fromType = InstType::i32;
    auto toType = InstType::u8;
    auto vdest = VReg(0);
    auto vreg = VReg(1);
    auto *instr = instructionBuilder.BuildCast(fromType, toType, vdest, vreg);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CAST);
    ASSERT_EQ(instr->GetRegType(), fromType);
    ASSERT_EQ(instr->GetTargetType(), toType);
    // ASSERT_EQ(instr->GetDestVReg(), vdest);
    // ASSERT_EQ(instr->GetVReg(), vreg);
}

TEST_F(InstructionsTest, TestCmp) {
    auto instType = InstType::u64;
    auto conditions = Conditions::LSTHAN;
    auto vreg1 = VReg(0);
    auto vreg2 = VReg(1);
    auto *instr =
        instructionBuilder.BuildCmp(instType, conditions, vreg1, vreg2);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CMP);
    ASSERT_EQ(instr->GetRegType(), instType);
    ASSERT_EQ(instr->GetCondCode(), conditions);
    // ASSERT_EQ(instr->GetVReg1(), vreg1);
    // ASSERT_EQ(instr->GetVReg2(), vreg2);
}

TEST_F(InstructionsTest, TestJa) {
    auto dest = 42;
    auto *instr = instructionBuilder.BuildJa(dest);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::JA);
    ASSERT_EQ(instr->GetRegType(), InstType::i64);
    ASSERT_EQ(instr->GetImm(), dest);
}

TEST_F(InstructionsTest, TestJmp) {
    int64_t dest = 42;
    auto *instr = instructionBuilder.BuildJmp(dest);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::JMP);
    ASSERT_EQ(instr->GetRegType(), InstType::i64);
    ASSERT_EQ(instr->GetImm(), dest);
}

TEST_F(InstructionsTest, TestRet) {
    auto instType = InstType::u8;
    auto vreg = VReg(0);
    auto *instr = instructionBuilder.BuildRet(instType, vreg);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::RET);
    ASSERT_EQ(instr->GetRegType(), instType);
    // ASSERT_EQ(instr->GetVReg(), vreg);
}

TEST_F(InstructionsTest, TestPhi) {
    auto instType = InstType::u16;
    auto vdest = VReg(0);
    auto vreg1 = VReg(1);
    auto vreg2 = VReg(2);
    auto *instr = instructionBuilder.BuildPhi(instType, vdest, vreg1, vreg2);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::PHI);
    ASSERT_EQ(instr->GetRegType(), instType);
    // ASSERT_EQ(instr->GetDestVReg(), vdest);
    // ASSERT_EQ(instr->GetVReg1(), vreg1);
    // ASSERT_EQ(instr->GetVReg2(), vreg2);
}

} // namespace ir::tests

#include "helperBuilderFunctions.h"
#include "peepholes.h"
#include "constFolding.h"
#include "irGen.h"
#include "gtest/gtest.h"

namespace ir::tests {
class OPTIMIZATIONS : public ::testing::Test {
  public:
  OPTIMIZATIONS() = default;

    virtual void TearDown() {
        instructionBuilder.Clear();
        irGenerator.Clear();
    }

    InstructionBuilder instructionBuilder;
    IRGenerator irGenerator;
};

TEST_F(OPTIMIZATIONS, TestMul_1) {
    auto instType = InstType::u64;
    auto *arg = instructionBuilder.BuildArg(instType);
    auto imm = 0UL;
    auto *instr = instructionBuilder.BuildMuli(instType, arg, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::MULI);
    ASSERT_EQ(instr->GetRegType(), instType);
    instr->PrintSSA();

    auto ph = Peepholes(new Graph());
    ph.AnalysisInst(instr);
    instr->PrintSSA();
}

TEST_F(OPTIMIZATIONS, TestMul_2) {
    auto instType = InstType::u64;
    auto *arg = instructionBuilder.BuildArg(instType);
    auto imm = 1UL;
    auto *instr = instructionBuilder.BuildMuli(instType, arg, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::MULI);
    ASSERT_EQ(instr->GetRegType(), instType);
    instr->PrintSSA();

    auto ph = Peepholes(new Graph());
    ph.AnalysisInst(instr);
    instr->PrintSSA();
}

TEST_F(OPTIMIZATIONS, TestXor_1) {
    auto instType = InstType::u64;
    auto *arg = instructionBuilder.BuildArg(instType);
    auto imm = 0UL;
    auto *instr = instructionBuilder.BuildXori(instType, arg, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::XORI);
    ASSERT_EQ(instr->GetRegType(), instType);
    instr->PrintSSA();

    auto ph = Peepholes(new Graph());
    ph.AnalysisInst(instr);
    instr->PrintSSA();
}

TEST_F(OPTIMIZATIONS, TestXor_2) {
    auto instType = InstType::u64;
    auto *arg1 = instructionBuilder.BuildArg(instType);
    auto *arg2 = instructionBuilder.BuildArg(instType);
    auto *instr = instructionBuilder.BuildXor(instType, arg1, arg2);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::XOR);
    ASSERT_EQ(instr->GetRegType(), instType);
    instr->PrintSSA();

    auto ph = Peepholes(new Graph());
    ph.AnalysisInst(instr);
    instr->PrintSSA();
}

TEST_F(OPTIMIZATIONS, TestShr_1) {
    auto instType = InstType::u64;
    auto *arg = instructionBuilder.BuildArg(instType);
    auto imm = 0UL;
    auto *instr = instructionBuilder.BuildShri(instType, arg, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::SHRI);
    ASSERT_EQ(instr->GetRegType(), instType);
    instr->PrintSSA();

    auto ph = Peepholes(new Graph());
    ph.AnalysisInst(instr);
    instr->PrintSSA();
}

TEST_F(OPTIMIZATIONS, TestShr_2) {
    auto instType = InstType::u64;
    auto *arg = instructionBuilder.BuildArg(instType);
    auto imm = 20UL;
    auto *instr = instructionBuilder.BuildShri(instType, arg, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::SHRI);
    ASSERT_EQ(instr->GetRegType(), instType);
    instr->PrintSSA();

    auto ph = Peepholes(new Graph());
    ph.AnalysisInst(instr);
    instr->PrintSSA();
}


} // namespace ir::tests

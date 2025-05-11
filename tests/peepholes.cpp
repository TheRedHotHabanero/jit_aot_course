#include "optimizations/peepholes.h"
#include "testBase.h"

namespace ir::tests {
class PeepholesTest : public TestBase {
  public:
    void SetUp() override {
        TestBase::SetUp();
        pass = new Peepholes(GetGraph());
    }
    void TearDown() override {
        delete pass;
        TestBase::TearDown();
    }

  public:
    Peepholes *pass = nullptr;
};

static void compareInstructions(std::vector<SingleInstruction *> expected,
                                BB *bblock) {
    ASSERT_EQ(bblock->GetSize(), expected.size());
    size_t i = 0;
    for (auto *instr : *bblock) {
        ASSERT_EQ(instr, expected[i++]);
    }
}

TEST_F(PeepholesTest, TestXORWithZero) {
    auto opType = InstType::i32;
    auto *arg = GetInstructionBuilder()->BuildArg(opType);
    auto *zero = GetInstructionBuilder()->BuildConst(opType, 0);
    auto *xorInstr = GetInstructionBuilder()->BuildXor(opType, arg, zero);

    auto *bblock = GetGraph()->CreateEmptyBB();
    GetGraph()->SetFirstBB(bblock);

    GetInstructionBuilder()->PushBackInst(bblock, arg);
    GetInstructionBuilder()->PushBackInst(bblock, zero);
    GetInstructionBuilder()->PushBackInst(bblock, xorInstr);

    auto prevSize = bblock->GetSize();
    pass->Run();

    TestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);
}

TEST_F(PeepholesTest, TestSHR1) {
    // case:
    // v0 = ARG
    // v1 = 0
    // v2 = v1 >> v0
    // expected:
    // v2 is replaced with 0

    auto opType = InstType::i32;
    auto *arg = GetInstructionBuilder()->BuildArg(opType);
    auto *constZero = GetInstructionBuilder()->BuildConst(opType, 0);
    auto *shrInstr = GetInstructionBuilder()->BuildShr(opType, constZero, arg);
    auto *userInstr = GetInstructionBuilder()->BuildAddi(opType, shrInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBB();
    GetGraph()->SetFirstBB(bblock);

    GetInstructionBuilder()->PushBackInst(bblock, constZero);
    GetInstructionBuilder()->PushBackInst(bblock, arg);
    GetInstructionBuilder()->PushBackInst(bblock, shrInstr);
    GetInstructionBuilder()->PushBackInst(bblock, userInstr);

    auto prevSize = bblock->GetSize();
    pass->Run();

    TestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);
    auto *constZeroInstr = userInstr->GetInput(0).GetInstruction();
    ASSERT_TRUE(constZeroInstr->IsConst());
    auto *constInstr = static_cast<ConstInstr *>(constZeroInstr);
    ASSERT_EQ(constInstr->GetValue(), 0);

    compareInstructions({constZero, arg, userInstr}, bblock);
    ASSERT_EQ(userInstr->GetInput(0), constZero);
}

TEST_F(PeepholesTest, TestSHR2) {
    // case:
    // v0 = ARG
    // v1 = 0
    // v2 = v0 >> v1
    // expected:
    // v2 is replaced with v0

    auto opType = InstType::i32;
    auto *arg = GetInstructionBuilder()->BuildArg(opType);
    auto *constZero = GetInstructionBuilder()->BuildConst(opType, 0);
    auto *shrInstr = GetInstructionBuilder()->BuildShr(opType, arg, constZero);
    auto *userInstr = GetInstructionBuilder()->BuildAddi(opType, shrInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBB();
    GetGraph()->SetFirstBB(bblock);
    GetInstructionBuilder()->PushBackInst(bblock, arg);
    GetInstructionBuilder()->PushBackInst(bblock, constZero);
    GetInstructionBuilder()->PushBackInst(bblock, shrInstr);
    GetInstructionBuilder()->PushBackInst(bblock, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    TestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);

    ASSERT_EQ(userInstr->GetInput(0), arg);
    compareInstructions({arg, constZero, userInstr}, bblock);
    ASSERT_EQ(bblock->GetLastInstBB(), userInstr);
}

TEST_F(PeepholesTest, TestSHR3) {
    // case:
    // v0 = ARG
    // v1 = 33
    // v2 = v0 >> v1
    // expected:
    // v2 is replaced with 0

    auto opType = InstType::i32;
    auto *arg = GetInstructionBuilder()->BuildArg(opType);
    auto *constLarge =
        GetInstructionBuilder()->BuildConst(opType, 33); // n >= 32
    auto *shrInstr = GetInstructionBuilder()->BuildShr(opType, arg, constLarge);
    auto *userInstr = GetInstructionBuilder()->BuildAddi(opType, shrInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBB();
    GetGraph()->SetFirstBB(bblock);
    GetInstructionBuilder()->PushBackInst(bblock, arg);
    GetInstructionBuilder()->PushBackInst(bblock, constLarge);
    GetInstructionBuilder()->PushBackInst(bblock, shrInstr);
    GetInstructionBuilder()->PushBackInst(bblock, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    TestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);

    auto *constZero = userInstr->GetInput(0).GetInstruction();
    ASSERT_TRUE(constZero->IsConst());
    auto *constInstr = static_cast<ConstInstr *>(constZero);
    ASSERT_EQ(constInstr->GetValue(), 0);

    compareInstructions({arg, constLarge, userInstr}, bblock);
    ASSERT_EQ(userInstr->GetInput(0), constZero);
}

TEST_F(PeepholesTest, TestMUL1) {
    // case:
    // v0 = ARG
    // v1 = 1
    // v2 = v0 * v1
    // expected:
    // v2 is replaced with v0

    auto opType = InstType::i32;
    auto *arg = GetInstructionBuilder()->BuildArg(opType);
    auto *constOne = GetInstructionBuilder()->BuildConst(opType, 1);
    auto *mulInstr = GetInstructionBuilder()->BuildMul(opType, arg, constOne);
    auto *userInstr = GetInstructionBuilder()->BuildAddi(opType, mulInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBB();
    GetGraph()->SetFirstBB(bblock);
    GetInstructionBuilder()->PushBackInst(bblock, arg);
    GetInstructionBuilder()->PushBackInst(bblock, constOne);
    GetInstructionBuilder()->PushBackInst(bblock, mulInstr);
    GetInstructionBuilder()->PushBackInst(bblock, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    TestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);

    ASSERT_EQ(userInstr->GetInput(0), arg);
    compareInstructions({arg, constOne, userInstr}, bblock);
    ASSERT_EQ(bblock->GetLastInstBB(), userInstr);
}

TEST_F(PeepholesTest, TestMUL2) {
    // case:
    // v0 = ARG
    // v1 = 0
    // v2 = v0 * v1
    // expected:
    // v2 is replaced with 0

    auto opType = InstType::i32;
    auto *arg = GetInstructionBuilder()->BuildArg(opType);
    auto *constZero = GetInstructionBuilder()->BuildConst(opType, 0);
    auto *mulInstr = GetInstructionBuilder()->BuildMul(opType, arg, constZero);
    auto *userInstr = GetInstructionBuilder()->BuildAddi(opType, mulInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBB();
    GetGraph()->SetFirstBB(bblock);
    GetInstructionBuilder()->PushBackInst(bblock, arg);
    GetInstructionBuilder()->PushBackInst(bblock, constZero);
    GetInstructionBuilder()->PushBackInst(bblock, mulInstr);
    GetInstructionBuilder()->PushBackInst(bblock, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    TestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);

    auto *constZeroInstr = userInstr->GetInput(0).GetInstruction();
    ASSERT_TRUE(constZeroInstr->IsConst());
    auto *constInstr = static_cast<ConstInstr *>(constZeroInstr);
    ASSERT_EQ(constInstr->GetValue(), 0);

    compareInstructions({arg, constZero, userInstr}, bblock);
    ASSERT_EQ(userInstr->GetInput(0), constZero);
}

TEST_F(PeepholesTest, TestMUL3) {
    // case:
    // v0 = ARG
    // v1 = 0
    // v2 = v1 * v0
    // expected:
    // v2 is replaced with 0

    auto opType = InstType::i32;
    auto *arg = GetInstructionBuilder()->BuildArg(opType);
    auto *constZero = GetInstructionBuilder()->BuildConst(opType, 0);
    auto *mulInstr = GetInstructionBuilder()->BuildMul(opType, constZero, arg);
    auto *userInstr = GetInstructionBuilder()->BuildAddi(opType, mulInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBB();
    GetGraph()->SetFirstBB(bblock);
    GetInstructionBuilder()->PushBackInst(bblock, constZero);
    GetInstructionBuilder()->PushBackInst(bblock, arg);
    GetInstructionBuilder()->PushBackInst(bblock, mulInstr);
    GetInstructionBuilder()->PushBackInst(bblock, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    TestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);

    auto *constZeroInstr = userInstr->GetInput(0).GetInstruction();
    ASSERT_TRUE(constZeroInstr->IsConst());
    auto *constInstr = static_cast<ConstInstr *>(constZeroInstr);
    ASSERT_EQ(constInstr->GetValue(), 0);

    compareInstructions({constZero, arg, userInstr}, bblock);
    ASSERT_EQ(userInstr->GetInput(0), constZero);
}

} // namespace ir::tests
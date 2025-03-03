#include "testBase.h"
#include "optimizations/peepholes.h"

namespace ir::tests {
class PeepholesTest : public TestBase {
public:
    void SetUp() override {
        GetIRGenerator().CreateGraph();
        pass = new Peepholes(GetIRGenerator().GetGraph());
    }

public:
    Peepholes *pass = nullptr;
};

static void verifyControlAndDataFlowGraphs(BB *bblock) {
    ASSERT_NE(bblock, nullptr);
    auto *instr = bblock->GetFirstInstBB();
    ASSERT_NE(instr, nullptr);
    while (instr) {
        if (instr != bblock->GetFirstInstBB()) {
            ASSERT_NE(instr->GetPrevInst(), nullptr);
        }
        if (instr != bblock->GetLastInstBB()) {
            ASSERT_NE(instr->GetNextInst(), nullptr);
        }

        if (instr->HasInputs()) {
            auto *typed = static_cast<InputsInstr *>(instr);
            bool found = false;
            for (size_t i = 0, end_idx = typed->GetInputsCount(); i < end_idx; ++i) {
                auto currUsers = typed->GetInput(i)->GetUsers();
                auto iter = std::find(currUsers.begin(), currUsers.end(), instr);
                if (iter != currUsers.end()) {
                    found = true;
                    break;
                }
            }
            ASSERT_TRUE(found);
        }

        auto inputUsers = instr->GetUsers();
        for (auto &&user : inputUsers) {
            auto *typed = static_cast<InputsInstr *>(user);
            bool found = false;
            for (size_t i = 0, end_idx = typed->GetInputsCount(); i < end_idx; ++i) {
                if (typed->GetInput(i) == instr) {
                    found = true;
                    break;
                }
            }
            ASSERT_TRUE(found);
        }

        if (instr->GetNextInst() == nullptr) {
            ASSERT_EQ(instr, bblock->GetLastInstBB());
        }
        instr = instr->GetNextInst();
    }
}

static void compareInstructions(std::vector<SingleInstruction *> expected, BB *bblock) {
    ASSERT_EQ(bblock->GetSize(), expected.size());
    size_t i = 0;
    for (auto *instr : *bblock) {
        ASSERT_EQ(instr, expected[i++]);
    }
}

TEST_F(PeepholesTest, TestXORWithZero) {
    auto opType = InstType::i32;
    auto *arg = GetInstructionBuilder().BuildArg(opType);
    auto *zero = GetInstructionBuilder().BuildConst(opType, 0);
    auto *xorInstr = GetInstructionBuilder().BuildXor(opType, arg, zero);

    auto *bblock = GetIRGenerator().CreateEmptyBB();
    GetIRGenerator().GetGraph()->SetFirstBB(bblock);

    GetInstructionBuilder().PushBackInst(bblock, arg);
    GetInstructionBuilder().PushBackInst(bblock, zero);
    GetInstructionBuilder().PushBackInst(bblock, xorInstr);

    auto prevSize = bblock->GetSize();
    pass->Run();

    verifyControlAndDataFlowGraphs(bblock);
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
    auto *arg = GetInstructionBuilder().BuildArg(opType);
    auto *constZero = GetInstructionBuilder().BuildConst(opType, 0);
    auto *shrInstr = GetInstructionBuilder().BuildShr(opType, constZero, arg); 
    auto *userInstr = GetInstructionBuilder().BuildAddi(opType, shrInstr, 123);

    auto *bblock = GetIRGenerator().CreateEmptyBB();
    GetIRGenerator().GetGraph()->SetFirstBB(bblock);

    GetInstructionBuilder().PushBackInst(bblock, constZero);
    GetInstructionBuilder().PushBackInst(bblock, arg);
    GetInstructionBuilder().PushBackInst(bblock, shrInstr);
    GetInstructionBuilder().PushBackInst(bblock, userInstr);

    auto prevSize = bblock->GetSize();
    pass->Run();

    verifyControlAndDataFlowGraphs(bblock);
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
    auto *arg = GetInstructionBuilder().BuildArg(opType);
    auto *constZero = GetInstructionBuilder().BuildConst(opType, 0);
    auto *shrInstr = GetInstructionBuilder().BuildShr(opType, arg, constZero);
    auto *userInstr = GetInstructionBuilder().BuildAddi(opType, shrInstr, 123);

    auto *bblock = GetIRGenerator().CreateEmptyBB();
    GetIRGenerator().GetGraph()->SetFirstBB(bblock);
    GetInstructionBuilder().PushBackInst(bblock, arg);
    GetInstructionBuilder().PushBackInst(bblock, constZero);
    GetInstructionBuilder().PushBackInst(bblock, shrInstr);
    GetInstructionBuilder().PushBackInst(bblock, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    verifyControlAndDataFlowGraphs(bblock);
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
    auto *arg = GetInstructionBuilder().BuildArg(opType);
    auto *constLarge = GetInstructionBuilder().BuildConst(opType, 33); // n >= 32
    auto *shrInstr = GetInstructionBuilder().BuildShr(opType, arg, constLarge);
    auto *userInstr = GetInstructionBuilder().BuildAddi(opType, shrInstr, 123);

    auto *bblock = GetIRGenerator().CreateEmptyBB();
    GetIRGenerator().GetGraph()->SetFirstBB(bblock);
    GetInstructionBuilder().PushBackInst(bblock, arg);
    GetInstructionBuilder().PushBackInst(bblock, constLarge);
    GetInstructionBuilder().PushBackInst(bblock, shrInstr);
    GetInstructionBuilder().PushBackInst(bblock, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    verifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);

    // Проверка, что SHR заменяется на 0
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
    auto *arg = GetInstructionBuilder().BuildArg(opType);
    auto *constOne = GetInstructionBuilder().BuildConst(opType, 1);
    auto *mulInstr = GetInstructionBuilder().BuildMul(opType, arg, constOne);
    auto *userInstr = GetInstructionBuilder().BuildAddi(opType, mulInstr, 123);

    auto *bblock = GetIRGenerator().CreateEmptyBB();
    GetIRGenerator().GetGraph()->SetFirstBB(bblock);
    GetInstructionBuilder().PushBackInst(bblock, arg);
    GetInstructionBuilder().PushBackInst(bblock, constOne);
    GetInstructionBuilder().PushBackInst(bblock, mulInstr);
    GetInstructionBuilder().PushBackInst(bblock, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    verifyControlAndDataFlowGraphs(bblock);
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
    auto *arg = GetInstructionBuilder().BuildArg(opType);
    auto *constZero = GetInstructionBuilder().BuildConst(opType, 0);
    auto *mulInstr = GetInstructionBuilder().BuildMul(opType, arg, constZero);
    auto *userInstr = GetInstructionBuilder().BuildAddi(opType, mulInstr, 123);

    auto *bblock = GetIRGenerator().CreateEmptyBB();
    GetIRGenerator().GetGraph()->SetFirstBB(bblock);
    GetInstructionBuilder().PushBackInst(bblock, arg);
    GetInstructionBuilder().PushBackInst(bblock, constZero);
    GetInstructionBuilder().PushBackInst(bblock, mulInstr);
    GetInstructionBuilder().PushBackInst(bblock, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    verifyControlAndDataFlowGraphs(bblock);
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
    auto *arg = GetInstructionBuilder().BuildArg(opType);
    auto *constZero = GetInstructionBuilder().BuildConst(opType, 0);
    auto *mulInstr = GetInstructionBuilder().BuildMul(opType, constZero, arg);
    auto *userInstr = GetInstructionBuilder().BuildAddi(opType, mulInstr, 123);

    auto *bblock = GetIRGenerator().CreateEmptyBB();
    GetIRGenerator().GetGraph()->SetFirstBB(bblock);
    GetInstructionBuilder().PushBackInst(bblock, constZero);
    GetInstructionBuilder().PushBackInst(bblock, arg);
    GetInstructionBuilder().PushBackInst(bblock, mulInstr);
    GetInstructionBuilder().PushBackInst(bblock, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    verifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);

    auto *constZeroInstr = userInstr->GetInput(0).GetInstruction();
    ASSERT_TRUE(constZeroInstr->IsConst());
    auto *constInstr = static_cast<ConstInstr *>(constZeroInstr);
    ASSERT_EQ(constInstr->GetValue(), 0);

    compareInstructions({constZero, arg, userInstr}, bblock);
    ASSERT_EQ(userInstr->GetInput(0), constZero);
}

}
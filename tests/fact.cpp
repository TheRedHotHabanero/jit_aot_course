#include "helperBuilderFunctions.h"
#include "irGen.h"
#include "gtest/gtest.h"
#include <iostream>

namespace ir::tests {
class IRGenTest : public ::testing::Test {
  protected:
    void SetUp() override { irGenerator.CreateGraph(); }

    void TearDown() override {
        instrBuilder.Clear();
        irGenerator.Clear();
    }

    InstructionBuilder instrBuilder;
    IRGenerator irGenerator;
};

/*

void GenerateFactorialIR(IRGenerator& irGenerator, InstructionBuilder&
instrBuilder) {
    // Create the entry basic block
    auto *entry = irGenerator.CreateEmptyBB();

    // Create function arguments and local variables
    VReg n = VReg(0);  // Argument for the function (input)
    VReg res = VReg(1); // Result variable (res)
    VReg i = VReg(2);   // Loop variable (i)

    // Initialize res = 1
    instrBuilder.BuildMovi(InstType::u64, res, 1);
    entry->InsertSingleInstrAfter(instrBuilder.GetLastInst(), nullptr); // Add
the instruction to the block

    // Initialize i = 2
    instrBuilder.BuildMovi(InstType::u32, i, 2);
    entry->InsertSingleInstrAfter(instrBuilder.GetLastInst(), nullptr); // Add
the instruction to the block

    // Create the loop start block
    auto *loopStart = irGenerator.CreateEmptyBB();

    // Create the loop condition (i <= n)
    instrBuilder.BuildCmp(InstType::u32, Conditions::LSTHAN, i, n);

    // Handle jumps
    auto *loopBody = irGenerator.CreateEmptyBB();
    auto *exitBlock = irGenerator.CreateEmptyBB();

    // Jump instructions with int64_t offsets instead of BasicBlock pointers
    int64_t loopBodyDest = loopBody->GetId(); // Destination for loop body
    int64_t exitBlockDest = exitBlock->GetId(); // Destination for exit block

    auto *jumpIfFalse = instrBuilder.BuildJa(exitBlockDest); // If condition is
false, jump to exit loopStart->InsertSingleInstrAfter(jumpIfFalse, nullptr); //
Add the instruction to the loop start

    auto *jumpToLoopBody = instrBuilder.BuildJmp(loopBodyDest);
    loopStart->InsertSingleInstrAfter(jumpToLoopBody, nullptr); // Add the
instruction to the loop start

    // Process the loop body
    instrBuilder.BuildMul(InstType::u64, res, res, i); // Multiply res by i
    loopBody->InsertSingleInstrAfter(instrBuilder.GetLastInst(), nullptr); //
Add the instruction to the loop body

    instrBuilder.BuildAddi(InstType::u32, i, i, 1); // Increment i: i += 1
    loopBody->InsertSingleInstrAfter(instrBuilder.GetLastInst(), nullptr); //
Add the instruction to the loop body

    // Jump back to loop start
    auto *jumpBackToStart = instrBuilder.BuildJmp(loopStart->GetId());
    loopBody->InsertSingleInstrAfter(jumpBackToStart, nullptr); // Jump back to
loop start

    // Exit block: return res
    auto *returnInstr = instrBuilder.BuildRet(InstType::u64, res);
    exitBlock->InsertSingleInstrAfter(returnInstr, nullptr); // Return res
}

TEST_F(IRGenTest, TestFactorialIR) {
    // Generate IR for factorial of 4
    VReg n = VReg(0);
    instrBuilder.BuildMovi(InstType::u32, n, 4);  // Set n = 4

    // Call the function to generate IR
    GenerateFactorialIR(irGenerator, instrBuilder);

    // Get the final return instruction
    auto *returnInstr = instrBuilder.GetLastInst();  // Use the correct method
to retrieve

    // Check that the return instruction is of the right type
    ASSERT_NE(returnInstr, nullptr);
    ASSERT_EQ(returnInstr->GetOpcode(), Opcode::RET);

    // Check if the return value corresponds to 4! = 24
    ASSERT_EQ(returnInstr->GetVirtualReg().GetRegValue(), 24);
}
*/
} // namespace ir::tests

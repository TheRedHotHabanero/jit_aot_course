#include "helperBuilderFunctions.h"
#include "irGen.h"
#include "gtest/gtest.h"
#include <iostream>

namespace ir::tests {

class FactorialGraphTest : public ::testing::Test {
  public:
    virtual void SetUp() { irGenerator.CreateGraph(); }
    virtual void TearDown() {
        instructionBuilder.Clear();
        irGenerator.Clear();
    }

    InstructionBuilder instructionBuilder;
    IRGenerator irGenerator;

    void CreateFactorialIR(int n) {
        auto instType = InstType::i32;
        auto vdest = VReg(0);                            // Result
        auto vregN = VReg(1);                            // Counter
        auto vregNValue = VReg(2);                       // Value of n
        auto *bbInit = irGenerator.CreateEmptyBB();      // Initialization block
        auto *bbLoopStart = irGenerator.CreateEmptyBB(); // Loop start block
        auto *bbCond = irGenerator.CreateEmptyBB();      // Condition block
        auto *bbLoopBody = irGenerator.CreateEmptyBB();  // Loop body block
        auto *bbEnd = irGenerator.CreateEmptyBB();       // End block

        // Initialize result and counter
        instructionBuilder.PushBackInst(
            bbInit, instructionBuilder.BuildAddi(instType, vdest, VReg(0),
                                                 1)); // result = 1
        instructionBuilder.PushBackInst(
            bbInit, instructionBuilder.BuildAddi(instType, vregN, VReg(0),
                                                 1)); // counter = 1
        instructionBuilder.PushBackInst(
            bbInit, instructionBuilder.BuildAddi(instType, vregNValue, VReg(0),
                                                 n)); // n = input value

        // Link the initialization block to the loop start
        irGenerator.GetGraph()->AddBBAsSuccessor(bbInit, bbLoopStart);

        // Loop Start: Jump to condition
        irGenerator.GetGraph()->AddBBAsSuccessor(bbLoopStart, bbCond);

        // Condition block: Compare counter with nInstType::u32,
        // Conditions::LSTHAN
        instructionBuilder.PushBackInst(
            bbCond,
            instructionBuilder.BuildCmp(instType, Conditions::LSTHAN, vregN,
                                        vregNValue)); // Compare counter with n

        // If counter > n, jump to end
        auto *bbCondEnd = irGenerator.CreateEmptyBB();
        instructionBuilder.PushBackInst(
            bbCond, instructionBuilder.BuildJa(
                        bbCondEnd->GetId())); // Jump to end if counter > n

        // Loop Body: Multiply result by counter
        instructionBuilder.PushBackInst(
            bbLoopBody,
            instructionBuilder.BuildMul(instType, vdest, vdest,
                                        vregN)); // result *= counter

        // Increment counter
        instructionBuilder.PushBackInst(
            bbLoopBody, instructionBuilder.BuildAddi(instType, vregN, vregN,
                                                     1)); // counter++

        // Link the blocks
        irGenerator.GetGraph()->AddBBAsSuccessor(
            bbCond, bbLoopBody); // If condition passes, go to body
        irGenerator.GetGraph()->AddBBAsSuccessor(
            bbLoopBody, bbCond); // Loop back to condition
        irGenerator.GetGraph()->AddBBAsSuccessor(bbCondEnd,
                                                 bbEnd); // Exit to end block

        // Link end block (if necessary)
        // You may want to handle the result or do other processing here.
    }
};

// Test case for factorial calculation
TEST_F(FactorialGraphTest, TestFactorialIR) {
    int n = 5; // Calculate 5!
    CreateFactorialIR(n);

    // Print the generated IR
    irGenerator.GetGraph()->PrintSSA(); // Print the graph structure
}

} // namespace ir::tests

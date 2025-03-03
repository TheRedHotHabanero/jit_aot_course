#ifndef JIT_AOT_COURSE_TEST_BASE_H_
#define JIT_AOT_COURSE_TEST_BASE_H_

#include "irGen/compiler.h"
#include "gtest/gtest.h"


namespace ir::tests {
class TestBase : public ::testing::Test {
public:
    void SetUp() override {
        GetIRGenerator().CreateGraph();
    }

    void TearDown() override {
        GetIRGenerator().Clear();
    }

    IRGenerator &GetIRGenerator() {
        return compiler_.GetIRGenerator();
    }
    InstructionBuilder &GetInstructionBuilder() {
        return compiler_.GetInstructionBuilder();
    }

public:
    Compiler compiler_;
};
}   // namespace ir::tests

#endif
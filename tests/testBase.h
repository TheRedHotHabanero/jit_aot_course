#ifndef JIT_AOT_COURSE_TEST_BASE_H_
#define JIT_AOT_COURSE_TEST_BASE_H_

#include "helperBuilderFunctions.h"
#include "irGen/compiler.h"
#include "gtest/gtest.h"

namespace ir::tests {
class TestBase : public ::testing::Test {
  public:
    void SetUp() override { graph_ = compiler_.CreateNewGraph(); }

    void TearDown() override {
        compiler_.DeleteFunctionGraph(graph_->GetId());
        graph_ = nullptr;
    }

    Graph *GetGraph() { return graph_; }

    InstructionBuilder *GetInstructionBuilder(Graph *targetGraph = nullptr) {
        if (targetGraph == nullptr) {
            targetGraph = GetGraph();
        }
        return targetGraph->GetInstructionBuilder();
    }

    static void CompareInstructions(std::vector<SingleInstruction *> expected,
                                    BB *bblock) {
        ASSERT_EQ(bblock->GetSize(), expected.size());
        size_t i = 0;
        for (auto *instr : *bblock) {
            ASSERT_EQ(instr, expected[i++]);
        }
    }

    static void VerifyControlAndDataFlowGraphs(Graph *graph);
    static void VerifyControlAndDataFlowGraphs(BB *bblock);

  public:
    Compiler compiler_;

  protected:
    Graph *graph_ = nullptr;
};
} // namespace ir::tests
#endif
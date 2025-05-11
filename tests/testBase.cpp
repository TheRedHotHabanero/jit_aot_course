#include "testBase.h"
#include "domTree/dfo_rpo.h"
namespace ir::tests {

void TestBase::VerifyControlAndDataFlowGraphs(Graph *graph) {
    graph->ForEachBB([](BB *bblock) {
        if (!bblock->IsLastInGraph()) {
            VerifyControlAndDataFlowGraphs(bblock);
        }
    });
}

void TestBase::VerifyControlAndDataFlowGraphs(BB *bblock) {
    ASSERT_NE(bblock, nullptr);
    SingleInstruction *instr = bblock->GetFirstPhiBB();
    instr = instr ? instr : bblock->GetFirstInstBB();
    if (!instr) {
        return;
    }
    size_t counter = 0;
    while (instr) {
        if (instr != bblock->GetFirstPhiBB() &&
            instr != bblock->GetFirstInstBB()) {
            ASSERT_NE(instr->GetPrevInst(), nullptr);
        }
        if (instr != bblock->GetLastInstBB()) {
            ASSERT_NE(instr->GetNextInst(), nullptr);
        }

        if (instr->HasInputs()) {
            auto *typed = static_cast<InputsInstr *>(instr);
            bool found = false;
            for (size_t i = 0, end_idx = typed->GetInputsCount(); i < end_idx;
                 ++i) {
                auto currUsers = typed->GetInput(i)->GetUsers();
                auto iter =
                    std::find(currUsers.begin(), currUsers.end(), instr);
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
            for (size_t i = 0, end_idx = typed->GetInputsCount(); i < end_idx;
                 ++i) {
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
        ++counter;
        instr = instr->GetNextInst();
    }
    ASSERT_EQ(bblock->GetSize(), counter);
}
} // namespace ir::tests
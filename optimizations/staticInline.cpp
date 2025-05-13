#include "staticInline.h"
#include "domTree/dfo_rpo.h"
#include "helperBuilderFunctions.h"
#include "irGen/base.h"
#include <iostream>

namespace ir {
void StaticInline::Run() {
    auto rpoBBlocks = RPO(graph_);
    auto instructions_count = graph_->CountInstructions();
    if (instructions_count >= maxInstrsAfterInlining) {
        std::cout << "Skip function due to too much instructions: "
                  << instructions_count << std::endl;
    }

    for (auto *bblock : rpoBBlocks) {
        for (auto *instr : *bblock) {
            if (!instr->IsCall()) {
                continue;
            }

            auto *call = static_cast<CallInstr *>(instr);
            auto *calleeGraph =
                PossibleToInlineFunction(call, instructions_count);
            if (!calleeGraph) {
                continue;
            }
            auto *copyGraph = graph_->GetCompiler()->CopyGraph(
                calleeGraph, graph_->GetInstructionBuilder());
            DoInlining(call, copyGraph);
        }
    }
}

Graph *StaticInline::PossibleToInlineFunction(CallInstr *call,
                                              size_t callerInstrsCount) {
    if (call->IsInlined()) {
        std::cout << "Already inlined, skipping. id = " << call->GetCallTarget()
                  << std::endl;
        return nullptr;
    }

    // without a fully-functioning IRBuilder StaticInline must rely on compiler
    // already having a Graph for the callee function
    auto *callee = graph_->GetCompiler()->GetFunction(call->GetCallTarget());
    if (callee == nullptr) {
        std::cout << "No IR graph found, skipping. id = "
                  << call->GetCallTarget() << std::endl;
        return nullptr;
    }
    if (callee == graph_) {
        return nullptr;
    }

    auto size_ = callee->CountInstructions();
    if (size_ >= maxCalleeInstrs) {
        std::cout << "Too many instructions: " << size_ << " when limit is "
                  << maxCalleeInstrs << ". id = " << call->GetCallTarget()
                  << std::endl;
        return nullptr;
    }
    if (callerInstrsCount + size_ >= maxInstrsAfterInlining) {
        std::cout << "Too many instructions after inlining: "
                  << callerInstrsCount + size_ << " when limit is "
                  << maxInstrsAfterInlining << std::endl;
        return nullptr;
    }

    return callee;
}

void StaticInline::DoInlining(CallInstr *call, Graph *callee) {
    assert((call) && (callee));

    auto blocks = call->GetInstBB()->SplitAfterInstruction(call, false);
    PropagateArgs(call, callee);
    PropagateReturnValue(call, callee, blocks.second);
    call->GetInstBB()->SetInstructionAsDead(call);

    InlineReadyGraph(callee, blocks.first, blocks.second);
    std::cout << "Inlined function #" << callee->GetId() << std::endl;
}

void StaticInline::PropagateArgs(CallInstr *call, Graph *callee) {
    auto *bblock = callee->GetFirstBB();
    assert(bblock);
    auto *argInstr = bblock->GetFirstInstBB();
    for (auto &arg : call->GetInputs()) {
        assert((argInstr) && argInstr->GetOpcode() == Opcode::ARG);
        auto *nextInstr = argInstr->GetNextInst();
        argInstr->ReplaceInputInUsers(arg.GetInstruction());
        bblock->SetInstructionAsDead(argInstr);
        argInstr = nextInstr;
    }
}

void StaticInline::PropagateReturnValue(CallInstr *call, Graph *callee,
                                        BB *postCallBlock) {
    assert((postCallBlock) && (callee->GetLastBB()) &&
           !callee->GetLastBB()->GetPredecessors().empty());
    if (call->GetType() == InstType::VOID) {
        RemoveVoidReturns(callee);
        return;
    }

    auto lastBlockPreds = callee->GetLastBB()->GetPredecessors();
    SingleInstruction *newInputForUsers = nullptr;
    if (lastBlockPreds.size() > 1) {
        // in case of multiple returns in callee we must collect all of them
        // into a single PHI instruction, which will be used in caller
        auto *phiReturnValue =
            callee->GetInstructionBuilder()->BuildPhi(call->GetType());
        for (auto *pred : lastBlockPreds) {
            auto *instr = pred->GetLastInstBB();
            assert((instr) && instr->GetOpcode() == Opcode::RET);

            auto *retInstr = static_cast<RetInstr *>(instr);
            auto phiInput = retInstr->GetInput(0);
            phiReturnValue->AddPhiInput(phiInput, phiInput->GetInstBB());
            phiInput->ReplaceUser(retInstr, phiReturnValue);
            pred->SetInstructionAsDead(retInstr);
        }
        postCallBlock->PushInstForward(phiReturnValue);
        newInputForUsers = phiReturnValue;
    } else {
        assert(lastBlockPreds.size() == 1);
        auto *pred = lastBlockPreds[0];
        auto *instr = pred->GetLastInstBB();
        assert((instr) && instr->GetOpcode() == Opcode::RET);
        auto *retInstr = static_cast<RetInstr *>(instr);
        pred->SetInstructionAsDead(retInstr);
        newInputForUsers = retInstr->GetInput(0).GetInstruction();
    }
    call->ReplaceInputInUsers(newInputForUsers);
}

void StaticInline::RemoveVoidReturns(Graph *callee) {
    for (auto *bblock : callee->GetLastBB()->GetPredecessors()) {
        auto *lastInstr = bblock->GetLastInstBB();
        assert(lastInstr->GetOpcode() == Opcode::RETVOID);
        bblock->SetInstructionAsDead(lastInstr);
    }
}

void StaticInline::InlineReadyGraph(Graph *callee, BB *callBlock,
                                    BB *postCallBlock) {
    assert((callee) && (callBlock) && (postCallBlock));
    RelinkBBs(callBlock->GetGraph(), callee);
    graph_->ConnectBBs(callBlock, callee->GetFirstBB());

    auto *lastBlock = callee->GetLastBB();
    assert((lastBlock) && lastBlock->IsEmpty() &&
           !lastBlock->GetPredecessors().empty());
    for (auto *prevLast : lastBlock->GetPredecessors()) {
        prevLast->ReplaceSuccessor(lastBlock, postCallBlock);
        postCallBlock->AddPredecessors(prevLast);
    }
}

void StaticInline::RelinkBBs(Graph *callerGraph, Graph *calleeGraph) {
    assert((callerGraph) && (calleeGraph));
    auto doRelink = [&callerGraph](BB *bblock) {
        if (!bblock->IsLastInGraph()) {
            callerGraph->AddBB(bblock);
        }
    };
    calleeGraph->ForEachBB(doRelink);
}
} // namespace ir
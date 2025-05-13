#include "compiler.h"
#include "domTree/loop.h"
#include "graphHelper.h"

namespace ir {
Graph *Compiler::CreateNewGraph(InstructionBuilder *instrBuilder) {
    auto *graph =
        allocator_.template New<Graph>(this, &allocator_, instrBuilder);
    graph->SetId(functionsGraphs_.size());
    functionsGraphs_.push_back(graph);
    return graph;
}

// Depth first ordered graph copy algorithm implementation.
Graph *Compiler::CopyGraph(Graph *source, InstructionBuilder *instrBuilder) {
    assert((source) && (instrBuilder));
    GraphCopyHelper helper(source);
    auto help = helper.CreateCopy(CreateNewGraph(instrBuilder));
    return help;
}

// defined here after full declaration of Compiler's methods
void CopyInstruction(
    BB *targetBlock, SingleInstruction *orig,
    ArenaUnorderedMap<size_t, SingleInstruction *> *instrsTranslation) {
    assert((targetBlock) && (orig) && (instrsTranslation));

    auto it = instrsTranslation->find(orig->GetInstID());
    assert(it == instrsTranslation->end());
    auto *copy = orig->Copy(targetBlock);
    targetBlock->PushInstBackward(copy);
    instrsTranslation->insert(it, {orig->GetInstID(), copy});
}

BB *BB::Copy(
    Graph *targetGraph,
    ArenaUnorderedMap<size_t, SingleInstruction *> *instrsTranslation) {
    assert(targetGraph);
    auto *result = targetGraph->CreateEmptyBB();

    if (!IsEmpty()) {
        for (auto *instr : *this) {
            CopyInstruction(result, instr, instrsTranslation);
        }
        assert(result->GetFirstInstBB());
        assert(result->GetLastInstBB());
    }
    return result;
}

std::pair<BB *, BB *> BB::SplitAfterInstruction(SingleInstruction *instr,
                                                bool connectAfterSplit) {
    assert((instr) && instr->GetInstBB() == this);
    auto *nextInstr = instr->GetNextInst();
    assert(nextInstr);

    auto *graph = GetGraph();
    auto *newBBlock = graph->CreateEmptyBB();

    if (connectAfterSplit) {
        graph->ConnectBBs(this, newBBlock);
    }

    if (GetLoop()) {
        GetLoop()->AddBB(newBBlock);
        newBBlock->SetLoop(GetLoop());
    }
    for (auto *succ : GetSuccessors()) {
        succ->DeletePredecessors(this);
        graph->ConnectBBs(newBBlock, succ);
    }
    successors_.clear();

    instr->SetNextInst(nullptr);
    nextInstr->SetPrevInst(nullptr);
    for (auto *iter = nextInstr; iter != nullptr; iter = iter->GetNextInst()) {
        iter->SetBB(newBBlock);
        newBBlock->size_ += 1;
    }
    size_ -= newBBlock->size_;

    if (nextInstr->IsPhi()) {
        assert(instr->IsPhi());
        newBBlock->firstPhiBB_ = static_cast<PhiInstr *>(nextInstr);
        newBBlock->lastPhiBB_ = lastPhiBB_;
        lastPhiBB_ = static_cast<PhiInstr *>(instr);
        newBBlock->firstInstBB_ = firstInstBB_;
        newBBlock->lastInstBB_ = lastInstBB_;
        firstInstBB_ = nullptr;
        lastInstBB_ = nullptr;
    } else {
        newBBlock->firstInstBB_ = nextInstr;
        newBBlock->lastInstBB_ = lastInstBB_;
        lastInstBB_ = instr;
    }

    return {this, newBBlock};
}
} // namespace ir
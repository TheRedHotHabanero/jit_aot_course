#include "graph.h"
#include <algorithm>
#include <cstdlib>

namespace ir {

size_t Graph::CountInstructions() {
    size_t counter = 0;
    ForEachBB([&counter](BB *bblock) { counter += bblock->GetSize(); });
    return counter;
}

BB *Graph::CreateEmptyBB(bool isTerminal) {
    auto *bblock = allocator_->template New<BB>(this);
    AddBB(bblock);
    if (isTerminal) {
        if (!GetLastBB()) {
            SetLastBB(CreateEmptyBB(false));
        }
        ConnectBBs(bblock, GetLastBB());
    }
    return bblock;
}

void Graph::ConnectBBs(BB *lhs, BB *rhs) {
    assert((lhs) && (rhs));
    lhs->AddSuccessors(rhs);
    rhs->AddPredecessors(lhs);
}

void Graph::AddBB(BB *bb) {
    if (bb == nullptr) {
        std::cout << "[Graph Error] BB went nullptr (AddBB)." << std::endl;
        std::abort();
    }
    bb->SetId(BBs_.size()); // because it was the last one
    BBs_.push_back(bb);
    bb->SetGraph(this);
}

// AddBBAsPredecessor -> AddBBBefore
void Graph::AddBBBefore(BB *bb_before, BB *bb) {
    if (bb == nullptr || bb_before == nullptr) {
        std::cout << "[Graph Error] BB or bb_before went nullptr (AddBBBefore)"
                  << std::endl;
        std::abort();
    }
    if (bb_before->GetGraph() != this || bb->GetGraph() == nullptr) {
        std::cout << "[Graph Error] GetGraph error in 'AddBBBefore'."
                  << std::endl;
        std::abort();
    }
    bb->SetGraph(this);

    // add of the same part as succ
    for (auto *b : bb_before->GetPredecessors()) {
        b->DeleteSuccessors(bb_before);
        b->AddSuccessors(bb);
        bb->AddPredecessors(b);
    }
    bb_before->GetPredecessors().clear();
    bb_before->AddPredecessors(bb);
    bb->AddSuccessors(bb_before);
}

void Graph::SetBBAsDead(BB *bb) {
    SetBBAsDeadImpl(bb);
    if (bb != GetLastBB()) {
        DeletePredecessors(bb);
        DeleteSuccessors(bb);
    } else {
        bb->GetPredecessors().clear();
        bb->GetSuccessors().clear();
    }
}

void Graph::SetBBAsDeadImpl(BB *bblock) {
    assert((bblock) && bblock->GetGraph() == this);
    auto id = bblock->GetId();
    assert(id < BBs_.size() && BBs_[id] == bblock);
    BBs_[id] = nullptr;
    bblock->SetId(ir::INVALID_BB_ID);
    bblock->SetGraph(nullptr);
    ++deadInstrCounter_;
}

bool BB::IsFirstInGraph() { return GetGraph()->GetFirstBB() == this; }

bool BB::IsLastInGraph() { return GetGraph()->GetLastBB() == this; }

void Graph::CleanupUnusedBlocks() {
    ArenaVector<BB *> activeBlocks;
    activeBlocks.reserve(BBs_.size());

    // add alive blocks
    for (auto it = BBs_.begin(); it != BBs_.end(); ++it) {
        if (*it) {
            activeBlocks.emplace_back(*it);
        }
    }

    // upd id
    int blockId = 0;
    for (auto &block : activeBlocks) {
        block->SetId(blockId++);
    }

    BBs_ = std::move(activeBlocks);
}

void Graph::DeletePredecessors(BB *bb) {
    for (auto *b : bb->GetPredecessors()) {
        b->DeleteSuccessors(bb);
    }
    bb->GetPredecessors().clear();
}

void Graph::DeleteSuccessors(BB *bb) {
    for (auto *b : bb->GetSuccessors()) {
        b->DeletePredecessors(bb);
    }
    bb->GetSuccessors().clear();
}

void Graph::PrintSSA() {
    std::cout << "SSA Form of the Graph:" << std::endl;
    for (auto *block :
         GetBBs()) { // Assuming GetBasicBlocks() returns a list of basic blocks
        block->PrintSSA(); // Print each block in SSA format
    }
}

BB::BB(Graph *graph)
    : bbId_(INVALID_BB_ID), predecessors_(graph->GetAllocator()->ToSTL()),
      successors_(graph->GetAllocator()->ToSTL()), graph_(graph),
      dominated_(graph->GetAllocator()->ToSTL()) {}

} // namespace ir

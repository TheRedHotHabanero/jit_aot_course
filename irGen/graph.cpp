#include "graph.h"
#include <algorithm>
#include <cstdlib>

namespace ir {

void Graph::ConnectBBs(BB *lhs, BB *rhs) {
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

// void Graph::AddBBAsSuccessor(BB *newBB, BB *bb) {
//     if (bb == nullptr || newBB == nullptr) {
//         std::cout << "[BB Error] BB or newBB went nullptr (AddBBAsSuccessor)"
//                   << std::endl;
//         std::abort();
//     }
//     if (newBB->GetGraph() != this || bb->GetGraph() == nullptr) {
//         std::cout << "[Graph Error] GetGraph error in 'AddBBAsSuccessor'."
//                   << std::endl;
//         std::abort();
//     }
//     bb->SetGraph(this);
//
//     // end of the same part with pred
//     for (auto *b : newBB->GetSuccessors()) {
//         b->DeletePredecessors(newBB);
//         b->AddPredecessors(bb);
//         bb->AddSuccessors(b);
//     }
//     newBB->GetSuccessors().clear();
//     newBB->AddSuccessors(bb);
//     bb->AddPredecessors(newBB);
//     if (lastBB_ == newBB) {
//         lastBB_ = bb;
//     }
// }

void Graph::SetBBAsDead(BB *bb) {
    if (bb == nullptr) {
        std::cout << "[Graph Error] BB went nullptr (SetBBAsDead)."
                  << std::endl;
        std::abort();
    }
    if (bb->GetGraph() != this) {
        std::cout << "[Graph Error] BB went this (SetBBAsDead)." << std::endl;
        std::abort();
    }
    if (std::find(BBs_.begin(), BBs_.end(), bb) == BBs_.end()) {

        std::cout << "[Graph Error] Failed to find BasicBlock." << std::endl;
        std::abort();
    }
    auto id = bb->GetId();
    // if (id >= BBs_->size() || BBs_[id] != bb) {
    if (id >= BBs_.size()) {
        std::cout << "[Graph Error] Error in 'SetBBAsDead'" << std::endl;
        std::abort();
    }
    BBs_[id] = nullptr;
    bb->SetId(INVALID_BB_ID);
    if (bb == lastBB_) {
        SetLastBB(nullptr);
    }
    DeletePredecessors(bb);
    DeleteSuccessors(bb);
}

void Graph::CleanupUnusedBlocks() {
    std::vector<BB *> activeBlocks;
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

} // namespace ir

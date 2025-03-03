#include "loop.h"
#include <iostream>

namespace ir {

size_t Loop::GetId() const { return id_; }

BB *Loop::GetHeader() { return header_; }

const BB *Loop::GetHeader() const { return header_; }

void Loop::AddBackEdge(BB *backEdgeSource) {
    if (std::find(backEdges_.begin(), backEdges_.end(), backEdgeSource) !=
        backEdges_.end()) {
        std::cout << "[Loop Error] AddBackEdge failed" << std::endl;
        std::abort();
    }
    backEdges_.push_back(backEdgeSource);
}

ArenaVector<BB *> Loop::GetBackEdges() { return backEdges_; }

ArenaVector<BB *> Loop::GetBasicBlocks() { return basicBlocks_; }

void Loop::AddBB(BB *bblock) {
    if (!(bblock) || (bblock->GetLoop() != nullptr) ||
        !(std::find(basicBlocks_.begin(), basicBlocks_.end(), bblock) ==
          basicBlocks_.end())) {
        std::cout << "[Loop Error] while adding basic block" << std::endl;
        std::abort();
    }
    bblock->SetLoop(this);
    basicBlocks_.push_back(bblock);
}

Loop *Loop::GetOuterLoop() { return outerLoop_; }

void Loop::SetOuterLoop(Loop *loop) { outerLoop_ = loop; }

const ArenaVector<Loop *> &Loop::GetInnerLoops() const { return innerLoops_; }

void Loop::AddInnerLoop(Loop *loop) {
    if (std::find(innerLoops_.begin(), innerLoops_.end(), loop) !=
        innerLoops_.end()) {
        std::cout << "[Loop Error] Inner loop already exists" << std::endl;
        std::abort();
    }
    innerLoops_.push_back(loop);
}

void Loop::SetIrreducibility(bool isIrr) { isIrreducible_ = isIrr; }

bool Loop::IsIrreducible() const { return isIrreducible_; }

bool Loop::IsRoot() const { return isRoot_; }

} // namespace ir

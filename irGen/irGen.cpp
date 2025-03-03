#include "irGen.h"
#include <iostream>

namespace ir {
class BB;
class Graph;

BB *IRGenerator::CreateEmptyBB() {
    auto *bb = allocator_->template New<BB>(graph_);
    BBs_.push_back(bb);
    graph_->AddBB(bb);
    return bb;
}

void IRGenerator::Clear() noexcept {
    graph_ = nullptr;
}

} // namespace ir
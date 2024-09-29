#include "irGen.h"
#include <iostream>

namespace ir {
class BB;
class Graph;

BB *IRGenerator::CreateEmptyBB() {
    auto *bb = new BB();
    BBs_.push_back(bb);
    if (graph_ != nullptr) {
        graph_->AddBB(bb);
    }
    return bb;
}

void IRGenerator::Clear() noexcept {
    // Deleting bb
    for (auto *bb : BBs_) {
        delete bb;
    }
    BBs_.clear();

    // Deleting graph
    delete graph_;
    graph_ = nullptr;
}

} // namespace ir
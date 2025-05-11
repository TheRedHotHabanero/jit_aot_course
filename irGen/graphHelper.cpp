#include "graphHelper.h"
#include <cassert>

namespace ir {
Graph *GraphCopyHelper::CreateCopy(Graph *copyTarget) {
    assert((copyTarget) && copyTarget->IsEmpty());
    reset(copyTarget);
    DfoCopy(source_->GetFirstBB());
    assert(target_->GetBBCount() == source_->GetBBCount());
    fixDFG();
    return target_;
}

void GraphCopyHelper::reset(Graph *copyTarget) {
    assert(copyTarget);
    target_ = copyTarget;
    auto *allocator = copyTarget->GetAllocator();
    instrsTranslation_ =
        allocator->NewUnorderedMap<size_t, SingleInstruction *>();
    visited_ = allocator->NewUnorderedMap<size_t, BB *>();
}

void GraphCopyHelper::DfoCopy(BB *currentBBlock) {
    assert((currentBBlock) && !visited_->contains(currentBBlock->GetId()));
    auto *bblockCopy = currentBBlock->Copy(target_, instrsTranslation_);
    if (currentBBlock == source_->GetFirstBB()) {
        target_->SetFirstBB(bblockCopy);
    }
    if (currentBBlock == source_->GetLastBB()) {
        target_->SetLastBB(bblockCopy);
    }
    visited_->insert({currentBBlock->GetId(), bblockCopy});

    for (auto *succ : currentBBlock->GetSuccessors()) {
        auto succCopyIter = visited_->find(succ->GetId());
        if (succCopyIter != visited_->end()) {
            target_->ConnectBBs(bblockCopy, succCopyIter->second);
        } else {
            DfoCopy(succ);
            target_->ConnectBBs(bblockCopy, visited_->at(succ->GetId()));
        }
    }
}

void GraphCopyHelper::fixDFG() {
    assert(target_->CountInstructions() == instrsTranslation_->size());
    auto *translation = instrsTranslation_;

    target_->ForEachBB([translation](BB *bblock) {
        assert(bblock);
        std::for_each(
            bblock->begin(), bblock->end(),
            [translation](SingleInstruction *instr) {
                // set correct inputs
                if (instr->HasInputs()) {
                    auto *withInputs = static_cast<InputsInstr *>(instr);
                    for (size_t i = 0, end = withInputs->GetInputsCount();
                         i < end; ++i) {
                        auto &input = withInputs->GetInput(i);
                        withInputs->SetInput(
                            translation->at(input->GetInstID()), i);
                    }
                }

                // set correct users
                memory::ArenaVector<SingleInstruction *> newUsers(
                    translation->get_allocator());
                newUsers.reserve(instr->UsersCount());
                for (auto *user : instr->GetUsers()) {
                    newUsers.push_back(translation->at(user->GetInstID()));
                }
                instr->SetNewUsers(std::move(newUsers));
            });
    });
}
} // namespace ir
#include "livenessAnalyzer.h"
#include "loopChecker.h"
#include <algorithm>

namespace ir {
bool LivenessAnalyzer::Run() {
    ResetRanges();
    OrderBBs();
    std::for_each(linearOrderedBlocks_.rbegin(), linearOrderedBlocks_.rend(),
                  [this](auto &i) { CalcLiveRanges(i); });
    return true;
}

void LivenessAnalyzer::ResetRanges() {
    rangeBegin = 0;
    linearNumber = 0;
}

bool LivenessAnalyzer::UnvisitedEdges(BB *bblock) {
    assert(bblock);
    if (bblock->GetLoop()->IsIrreducible()) {
        return false;
    }

    auto preds = bblock->GetPredecessors();
    return !bblock->IsLoopHeader();
}

void LivenessAnalyzer::CalcLiveRanges(BlockInfo &info) {
    auto *bblock = info.GetBlock(); // check null
    auto blockRange = info.GetRange();
    auto &liveSet = info.GetLiveSet();
    auto blockRangeBegin = blockRange.GetBegin();
    for (auto *instr = bblock->GetLastInstBB();
         instr != nullptr && !instr->IsPhi(); instr = instr->GetPrevInst()) {
        auto &intervals = getLiveIntervals(instr);
        auto liveNumber = 1; // need to set live number
        // intervals set the beginning with live number
        liveSet.Remove(instr);
    }
}

void LivenessAnalyzer::CalcInitialLiveSet(BlockInfo &info) const {
    auto &liveSet = info.GetLiveSet();
    auto *bblock = info.GetBlock();

    for (auto *succ : bblock->GetSuccessors()) {
        // iterating with phi
    }
}
} // namespace ir
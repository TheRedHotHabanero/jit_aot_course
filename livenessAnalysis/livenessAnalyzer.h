#ifndef JIT_AOT_COURSE_LIVENESS_ANALYSIS_LIVENESS_ANALYZER_H_
#define JIT_AOT_COURSE_LIVENESS_ANALYSIS_LIVENESS_ANALYZER_H_

#include "liveRanges.h"
#include <list>

namespace ir {
class LivenessAnalyzer {
  public:
    ~LivenessAnalyzer() = default;

    bool Run();

    std::vector<LiveRanges> &GetLiveIntervals() { return liveRanges_; }
    const std::vector<LiveRanges> &GetLiveIntervals() const {
        return liveRanges_;
    }

  private:
    void ResetRanges();
    void OrderBBs();
    bool UnvisitedEdges(BB *bblock);

    LiveRange::RangeType OrderInstrs(BB *bblock);

    void CalcLiveRanges(BlockInfo &info);
    void CalcInitialLiveSet(BlockInfo &info) const;

    LiveRanges &getLiveIntervals(SingleInstruction *instr) {
        return liveRanges_[1]; /*get number for instr*/
    }

  private:
    std::vector<BlockInfo> linearOrderedBlocks_;
    std::vector<LiveRanges> liveRanges_;

    LiveRange::RangeType rangeBegin = 0;
    size_t linearNumber = 0;
};
} // namespace ir

#endif // JIT_AOT_COURSE_LIVENESS_ANALYSIS_LIVENESS_ANALYZER_H_
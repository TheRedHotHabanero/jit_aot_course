#include "graph.h"
#include "singleInstruction.h"
#include <algorithm>
#include <cassert>
#include <ostream>
#include <unordered_set>
#include <vector>

namespace ir {

class LiveRange {
  public:
    using RangeType = int;

    LiveRange(RangeType begin, RangeType end) : begin_(begin), end_(end) {}

    RangeType GetBegin() { return begin_; }
    RangeType GetEnd() { return end_; }

  private:
    RangeType begin_;
    RangeType end_;
};

class LiveRanges {
  public:
    void AddRange(LiveRange::RangeType begin, LiveRange::RangeType end) {
        ExtendRange(LiveRange(begin, end));
    }

    void ExtendRange(const LiveRange &rng) { ranges_.push_back(rng); }

    SingleInstruction *GetInstruction() const { return instr_; }

    std::vector<LiveRange> &GetRanges() { return ranges_; }

  private:
    std::vector<LiveRange> ranges_;
    SingleInstruction *instr_ = nullptr;
};

class LiveSet {
  public:
    LiveSet() = default;
    LiveSet(const LiveSet &other) : instructions(other.instructions) {}
    LiveSet(LiveSet &&other) noexcept
        : instructions(std::move(other.instructions)) {}
    ~LiveSet() = default;

    void Add(SingleInstruction *instr) {
        assert(instr);
        instructions.insert(instr);
    }

    bool Remove(SingleInstruction *instr) {
        assert(instr);
        return instructions.erase(instr);
    }

    LiveSet &Union(LiveSet &&other) {
        instructions.merge(std::move(other.instructions));
        return *this;
    }

    auto begin() { return instructions.begin(); }
    auto end() { return instructions.end(); }

  private:
    std::unordered_set<SingleInstruction *> instructions;
};

class BlockInfo {
  public:
    explicit BlockInfo() : bblock_(nullptr), liveRange_(0, 0) {}
    BlockInfo(BB *b, LiveRange range) : bblock_(b), liveRange_(range) {}

    BlockInfo(const BlockInfo &other)
        : bblock_(other.bblock_), liveRange_(other.liveRange_),
          liveSet_(other.liveSet_) {}

    ~BlockInfo() = default;
    BB *GetBlock() { return bblock_; }
    void SetBlock(BB *b) { bblock_ = b; }
    LiveRange GetRange() { return liveRange_; }
    void SetRange(LiveRange range) { liveRange_ = range; }
    LiveSet &GetLiveSet() { return liveSet_; }

  private:
    BB *bblock_;
    LiveRange liveRange_;
    LiveSet liveSet_;
};

} // namespace ir
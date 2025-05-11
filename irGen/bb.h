/*
    Basic block structure is stolen from lecture


    Basic block       Basic block       Basic block
         |                 |                 |
         |                 |                 |
         |____________Basic block____________|
                    ________________
                    | preds(many!) |
                    | succs(<=2)   |
                    | first phi    |
                    | first inst   |
                    | last inst    |
                    | graph        |
                    |______________|
                           |
                           |
                           |
                      Basic Block
                           |
                           |
    Basic Block <----------------------> Basic Block
*/
#ifndef JIT_AOT_COURSE_IR_GEN_BB_H_
#define JIT_AOT_COURSE_IR_GEN_BB_H_

#include "domTree/arena.h"
#include "instructions.h"
#include "singleInstruction.h"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

namespace ir {
class Graph;
class Loop;
static constexpr size_t INVALID_BB_ID = static_cast<size_t>(-1);

using namespace memory;
class BB : public Markable {
  public:
    BB(Graph *graph);

    BB(const BB &) = delete;
    BB &operator=(const BB &) = delete;
    BB(BB &&) = delete;
    BB &operator=(BB &&) = delete;
    virtual ~BB() = default;

  public:
    // getters for all in private section
    size_t GetId() { return bbId_; }
    bool IsEmpty() const { return GetSize() == 0; }
    bool IsFirstInGraph();
    bool IsLastInGraph();
    bool HasNoSuccessors() const {
      return successors_.empty();
    }
    bool HasNoPredecessors() const { return predecessors_.empty(); }
    PhiInstr *GetFirstPhiBB() { return firstPhiBB_; }
    memory::ArenaVector<BB *> &GetPredecessors() { return predecessors_; }
    memory::ArenaVector<BB *> &GetSuccessors() { return successors_; }
    SingleInstruction *GetFirstInstBB() { return firstInstBB_; }
    SingleInstruction *GetLastInstBB() { return lastInstBB_; }
    PhiInstr *GetLastPhiBB() { return lastPhiBB_; }
    const PhiInstr *GetLastPhiBB() const { return lastPhiBB_; }
    Graph *GetGraph() { return graph_; }
    BB *GetDominator() { return dominator_; }
    const BB *GetDominator() const { return dominator_; }
    memory::ArenaVector<BB *> &GetDominatedBBs() { return dominated_; }
    const memory::ArenaVector<BB *> &GetDominatedBBs() const {
        return dominated_;
    }
    bool Domites(const BB *bblock) const;
    Loop *GetLoop() { return loop_; }
    const Loop *GetLoop() const { return loop_; }

  public:
    void SetId(size_t id) { bbId_ = id; }
    void AddPredecessors(BB *bb);
    void DeletePredecessors(BB *bb);
    void AddSuccessors(BB *bb);
    void DeleteSuccessors(BB *bb);

    void InsertSingleInstrBefore(SingleInstruction *instToMove,
                                 SingleInstruction *currentInstr);
    void InsertSingleInstrAfter(SingleInstruction *instToInsert,
                                SingleInstruction *currentInstr);
    void SetGraph(Graph *newGraph) { graph_ = newGraph; }
    void SetInstructionAsDead(SingleInstruction *inst);
    void ReplaceInstruction(SingleInstruction *prevInstr,
                            SingleInstruction *newInstr);
    void ReplaceInDataFlow(SingleInstruction *prevInstr,
                           SingleInstruction *newInstr);
    void ReplaceSuccessor(BB *prevSucc, BB *newSucc);
    size_t GetSize() const { return size_; }

    std::pair<BB *, BB *> SplitAfterInstruction(SingleInstruction *instr,
                                                bool connectAfterSplit);

    BB *Copy(Graph *targetGraph,
             memory::ArenaUnorderedMap<size_t, SingleInstruction *>
                 *instrsTranslation_);
    template <bool PushBack> void PushInstruction(SingleInstruction *instr);
    void PushInstForward(SingleInstruction *instr);
    void PushInstBackward(SingleInstruction *instr);

    void PushPhi(SingleInstruction *instr);
    void SetDominator(BB *newIDom) { dominator_ = newIDom; }
    void AddDominatedBlock(BB *bblock) { dominated_.push_back(bblock); }
    void SetLoop(Loop *newLoop) { loop_ = newLoop; }
    void PrintSSA();

    void ReplaceInControlFlow(SingleInstruction *prevInstr,
                              SingleInstruction *newInstr);

  public:
    // STL compatible iterator
    template <typename T> class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T;
        using reference = T;

        explicit Iterator(T instr = nullptr) : curr(instr) {}

        reference operator*() const { return curr; }

        Iterator &operator++() {
            curr = curr->GetNextInst();
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator &other) const {
            return curr == other.curr;
        }
        bool operator!=(const Iterator &other) const {
            return !(*this == other);
        }

      private:
        T curr;
    };

    auto begin() {
        SingleInstruction *instr = GetFirstPhiBB();
        return Iterator{instr != nullptr ? instr : GetFirstInstBB()};
    }
    auto end() { return Iterator<decltype(GetLastInstBB())>{nullptr}; }
    auto size() const { return GetSize(); }

  private:
    // according to the scheme
    size_t bbId_ = ir::INVALID_BB_ID;
    size_t size_ = 0;
    PhiInstr *firstPhiBB_ = nullptr;
    PhiInstr *lastPhiBB_ = nullptr;
    memory::ArenaVector<BB *> predecessors_;
    memory::ArenaVector<BB *> successors_;
    SingleInstruction *firstInstBB_ = nullptr;
    SingleInstruction *lastInstBB_ = nullptr;
    BB *dominator_ = nullptr;
    Loop *loop_ = nullptr;
    Graph *graph_ = nullptr;
    memory::ArenaVector<BB *> dominated_;
};

static_assert(std::input_or_output_iterator<BB::Iterator<SingleInstruction *>>);
static_assert(std::ranges::range<BB>);

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_BB_H_

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

#include "instructions.h"
#include "singleInstruction.h"
#include "domTree/arena.h"
#include <iostream>
#include <vector>

namespace ir {
class Graph;
class Loop;
static constexpr size_t INVALID_BB_ID = static_cast<size_t>(-1);

class BB {
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
    PhiInstr *GetFirstPhiBB() { return firstPhiBB_; }
    memory::ArenaVector<BB *> &GetPredecessors() { return predecessors_; }
    memory::ArenaVector<BB *> &GetSuccessors() { return successors_; }
    SingleInstruction *GetFirstInstBB() { return firstInstBB_; }
    SingleInstruction *GetLastInstBB() { return lastInstBB_; }
    Graph *GetGraph() { return graph_; }
    BB *GetDominator() { return dominator_; }
    const BB *GetDominator() const { return dominator_; }
    memory::ArenaVector<BB *> &GetDominatedBBs() { return dominated_; }
    const memory::ArenaVector<BB *> &GetDominatedBBs() const { return dominated_; }
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
    void ReplaceInstruction(SingleInstruction *prevInstr, SingleInstruction *newInstr);
    void ReplaceInDataFlow(SingleInstruction *prevInstr, SingleInstruction *newInstr);
    size_t GetSize() const {
      return size_;
    }
    void PushInstForward(SingleInstruction *instr);
    void PushInstBackward(SingleInstruction *instr);

    void PushPhi(SingleInstruction *instr);
    void SetDominator(BB *newIDom) { dominator_ = newIDom; }
    void AddDominatedBlock(BB *bblock) { dominated_.push_back(bblock); }
    void SetLoop(Loop *newLoop) { loop_ = newLoop; }
    void PrintSSA();
    void operator delete([[maybe_unused]] void *unused1, [[maybe_unused]] void *unused2) noexcept {}    \
    void *operator new([[maybe_unused]] size_t size) = delete;                                          \
    void operator delete([[maybe_unused]] void *unused, [[maybe_unused]] size_t size) {                 \
        std::cerr << "UNREACHABLE" << std::endl;                                                                                \
    }                                                                                                   \
    void *operator new([[maybe_unused]] size_t size, void *ptr) noexcept {                              \
        return ptr;                                                                                     \
    }

    void replaceInControlFlow(SingleInstruction *prevInstr, SingleInstruction *newInstr);

public:
    // STL compatible iterator
    template <typename T>
    class Iterator {
    public:
        explicit Iterator(T instr) : curr(instr) {}
        Iterator &operator++() {
            curr = curr->GetNextInst();
            return *this;
        }
        Iterator operator++(int) {
            auto retval = *this;
            ++(*this);
            return retval;
        }
        bool operator==(const Iterator &other) const {
            return curr == other.curr;
        }
        bool operator!=(const Iterator &other) const {
            return !(*this == other);
        }
        T operator*() {
            return curr;
        }

        // iterator traits
        using difference_type = size_t;
        using value_type = size_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::forward_iterator_tag;

    private:
        T curr;
    };

    auto begin() {
        return Iterator{GetFirstInstBB()};
    }
    auto cbegin() {
        return Iterator{GetFirstInstBB()};
    }
    auto end() {
        return Iterator{GetLastInstBB()};
    }
    auto cend() {
        return Iterator{GetLastInstBB()};
    }

private:
    // according to the scheme
    size_t bbId_ = ir::INVALID_BB_ID;
    size_t size_ = 0;
    PhiInstr *firstPhiBB_ = nullptr;
    memory::ArenaVector<BB *> predecessors_;
    memory::ArenaVector<BB *> successors_;
    SingleInstruction *firstInstBB_ = nullptr;
    SingleInstruction *lastInstBB_ = nullptr;
    BB *dominator_ = nullptr;
    Loop *loop_ = nullptr;
    Graph *graph_ = nullptr;
    memory::ArenaVector<BB *> dominated_;
};

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_BB_H_

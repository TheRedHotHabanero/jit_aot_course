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
#include <iostream>
#include <vector>

namespace ir {
class Graph;
class Loop;
static const size_t INVALID_BB_ID = static_cast<size_t>(-1);

class BB {
  public:
    BB()
        : bbId_(INVALID_BB_ID), firstPhiBB_(nullptr), firstInstBB_(nullptr),
          lastInstBB_(nullptr), dominator_(nullptr), loop_(nullptr),
          graph_(nullptr){};
    BB(Graph *graph)
        : bbId_(INVALID_BB_ID), firstPhiBB_(nullptr), firstInstBB_(nullptr),
          lastInstBB_(nullptr), dominator_(nullptr), loop_(nullptr),
          graph_(graph){};
    BB(const BB &) = delete;
    BB &operator=(const BB &) = delete;
    BB(BB &&) = delete;
    BB &operator=(BB &&) = delete;
    virtual ~BB() = default;

  private:
    // according to the scheme
    size_t bbId_;
    PhiInstr *firstPhiBB_;
    std::vector<BB *> predecessors_;
    std::vector<BB *> successors_;
    SingleInstruction *firstInstBB_;
    SingleInstruction *lastInstBB_;
    BB *dominator_;
    Loop *loop_;
    Graph *graph_;
    std::vector<BB *> dominated_;

  public:
    // getters for all in private section
    size_t GetId() { return bbId_; }
    PhiInstr *GetFirstPhiBB() { return firstPhiBB_; }
    std::vector<BB *> &GetPredecessors() { return predecessors_; }
    std::vector<BB *> &GetSuccessors() { return successors_; }
    SingleInstruction *GetFirstInstBB() { return firstInstBB_; }
    SingleInstruction *GetLastInstBB() { return lastInstBB_; }
    Graph *GetGraph() { return graph_; }
    BB *GetDominator() { return dominator_; }
    const BB *GetDominator() const { return dominator_; }
    std::vector<BB *> &GetDominatedBBs() { return dominated_; }
    const std::vector<BB *> &GetDominatedBBs() const { return dominated_; }
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
    void ReplaceInstruction(SingleInstruction *oldInst, SingleInstruction *newInst);
    void PushInstForward(SingleInstruction *instr);
    void PushInstBackward(SingleInstruction *instr);

    void PushPhi(SingleInstruction *instr);
    void SetDominator(BB *newIDom) { dominator_ = newIDom; }
    void AddDominatedBlock(BB *bblock) { dominated_.push_back(bblock); }
    void SetLoop(Loop *newLoop) { loop_ = newLoop; }
    void PrintSSA();
};

} // namespace ir

#endif // #define JIT_AOT_COURSE_IR_GEN_BB_H_

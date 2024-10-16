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
static constexpr size_t const INVALID_BB_ID = -1;

class BB {
  public:
    BB()
        : bbId_(INVALID_BB_ID), firstPhiBB_(nullptr), firstInstBB_(nullptr),
          lastInstBB_(nullptr), graph_(nullptr){};
    explicit BB(Graph *graph)
        : bbId_(INVALID_BB_ID), firstPhiBB_(nullptr), firstInstBB_(nullptr),
          lastInstBB_(nullptr), graph_(graph){};
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
    Graph *graph_;

  public:
    // getters for all in private section
    size_t GetId() { return bbId_; }
    PhiInstr *GetFirstPhiBB() { return firstPhiBB_; }
    std::vector<BB *> &GetPredecessors() { return predecessors_; }
    std::vector<BB *> &GetSuccessors() { return successors_; }
    SingleInstruction *GetFirstInstBB() { return firstInstBB_; }
    SingleInstruction *GetLastInstBB() { return lastInstBB_; }
    Graph *GetGraph() { return graph_; }

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
    void PushInstForward(SingleInstruction *instr);
    void PushInstBackward(SingleInstruction *instr);

    void pushPhi(SingleInstruction *instr);
    void PrintSSA();
};

} // namespace ir

#endif // #define JIT_AOT_COURSE_IR_GEN_BB_H_

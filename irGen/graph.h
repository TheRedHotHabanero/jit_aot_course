/*
    Graph structure is stolen from lecture

                        Graph
        __________________________________________________________________________
       | Blocks             |
       | Method Info        |   keeps shared data between compiler/interpreter
       | Runtime Info       |   get info about runtime types
       | Optimiztions Info  |   keeps info about analyses and optimization
   pipeline | Backend info       |   arch-specific |
   ___________________|______________________________________________________
*/

#ifndef JIT_AOT_COURSE_IR_GEN_GRAPH_H_
#define JIT_AOT_COURSE_IR_GEN_GRAPH_H_

#include "bb.h"
#include <vector>
namespace ir {

class Graph {
  public:
    Graph() : firstBB_(nullptr), lastBB_(nullptr) {}
    Graph &operator=(const Graph &) = delete;
    Graph(const Graph &) = delete;
    Graph(Graph &&) = delete;
    Graph &operator=(Graph &&) = delete;
    virtual ~Graph() = default;

  public:
    BB *GetFirstBB() { return firstBB_; }
    BB *GetLastBB() { return lastBB_; }
    void SetFirstBB(BB *bb) { firstBB_ = bb; }
    void SetLastBB(BB *bb) { lastBB_ = bb; }
    std::vector<BB *> GetBBs() { return BBs_; }

  public:
    void AddBB(BB *bb);
    void SetBBAsDead(BB *bb);
    void AddBBBefore(BB *newBB, BB *bb);
    // void AddBBAsSuccessor(BB *newBB, BB *bb);
    void CleanupUnusedBlocks();
    void DeletePredecessors(BB *bb);
    void DeleteSuccessors(BB *bb);
    void UpdPhiInst();
    void PrintSSA();
    void ConnectBBs(BB *lhs, BB *rhs);

  private:
    BB *firstBB_;
    BB *lastBB_;
    std::vector<BB *> BBs_;
};
} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_GRAPH_H_

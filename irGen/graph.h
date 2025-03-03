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
#include "domTree/arena.h"
#include "helperBuilderFunctions.h"
#include <vector>
namespace ir {
using memory::ArenaAllocator;
using memory::ArenaVector;
class Loop;

class Graph {
  public:
    Graph(ArenaAllocator *allocator, InstructionBuilder* instrBuilder) : allocator_(allocator), firstBB_(nullptr), 
      lastBB_(nullptr), BBs_(allocator_->ToSTL()), loopTreeRoot_(nullptr), instrBuilder_(instrBuilder)  {}

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
    ArenaVector<BB *> GetBBs() { return BBs_; }
    Loop *GetLoopTree() { return loopTreeRoot_; }
    const Loop *GetLoopTree() const { return loopTreeRoot_; }
    ArenaAllocator *GetAllocator() const { return allocator_; }
    InstructionBuilder *GetInstructionBuilder() { return instrBuilder_; }
  public:
    void AddBB(BB *bb);
    void SetBBAsDead(BB *bb);
    void AddBBBefore(BB *newBB, BB *bb);
    // void AddBBAsSuccessor(BB *newBB, BB *bb);
    void SetLoopTree(Loop *loop) { loopTreeRoot_ = loop; }
    void CleanupUnusedBlocks();
    void DeletePredecessors(BB *bb);
    void DeleteSuccessors(BB *bb);
    void UpdPhiInst();
    void PrintSSA();
    void ConnectBBs(BB *lhs, BB *rhs);
    size_t GetBBCount() const { return BBs_.size(); }
    bool IsEmpty() const { return BBs_.empty(); }

  private:
    ArenaAllocator *allocator_;
    BB *firstBB_;
    BB *lastBB_;
    memory::ArenaVector<BB *> BBs_;
    Loop *loopTreeRoot_;
    InstructionBuilder *instrBuilder_;
};
} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_GRAPH_H_

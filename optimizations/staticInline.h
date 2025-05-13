#ifndef JIT_AOT_COURSE_STATICINLINE_H_
#define JIT_AOT_COURSE_STATICINLINE_H_

#include "graph.h"
#include "pass.h"

namespace ir {
class StaticInline : public OptimizationPassBase {
  public:
    StaticInline(Graph *graph, size_t maxCalleeInstrs,
                 size_t maxInstrsAfterInlining)
        : OptimizationPassBase(graph), maxCalleeInstrs(maxCalleeInstrs),
          maxInstrsAfterInlining(maxInstrsAfterInlining) {
        assert(maxCalleeInstrs < maxInstrsAfterInlining);
    }

    void Run() override;

  private:
    Graph *PossibleToInlineFunction(CallInstr *call, size_t callerInstrsCount);
    void DoInlining(CallInstr *call, Graph *callee);
    void PropagateArgs(CallInstr *call, Graph *callee);
    void PropagateReturnValue(CallInstr *call, Graph *callee,
                              BB *postCallBlock);
    void RemoveVoidReturns(Graph *callee);
    void InlineReadyGraph(Graph *callee, BB *callBlock, BB *postCallBlock);
    void RelinkBBs(Graph *callerGraph, Graph *calleeGraph);

  private:
    size_t maxCalleeInstrs;
    size_t maxInstrsAfterInlining;
};
} // namespace ir

#endif // JIT_AOT_COURSE_STATICINLINE_H_
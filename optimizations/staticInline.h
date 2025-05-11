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
    //~StaticInline() noexcept override = default;

    void Run() override;

  private:
    // Returns a pointer to graph to be inlined if inlining is feasible to do,
    // nullptr otherwise.
    Graph *PossibleToInlineFunction(CallInstr *call, size_t callerInstrsCount);

    void DoInlining(CallInstr *call, Graph *callee);

    // Connects caller's arguments directly into their usages in callee's DFG.
    void PropagateArgs(CallInstr *call, Graph *callee);

    // Connect callee's return value directly into its usages in caller's DFG.
    // Simply returns return in case of RETVOID
    void PropagateReturnValue(CallInstr *call, Graph *callee,
                              BB *postCallBlock);

    void RemoveVoidReturns(Graph *callee);
    void InlineReadyGraph(Graph *callee, BB *callBlock, BB *postCallBlock);

    // Links all callee's basic blocks to the resulting caller graph.
    void RelinkBBs(Graph *callerGraph, Graph *calleeGraph);

  private:
    static constexpr const char *PASS_NAME = "inlining";

  private:
    size_t maxCalleeInstrs;
    size_t maxInstrsAfterInlining;
};
} // namespace ir

#endif // JIT_AOT_COURSE_STATICINLINE_H_
#ifndef JIT_AOT_COURSE_CHECK_ELIMINATION_H_
#define JIT_AOT_COURSE_CHECK_ELIMINATION_H_

#include "pass.h"

namespace ir {
class CheckElimination : public OptimizationPassBase {
  public:
    explicit CheckElimination(Graph *graph) : OptimizationPassBase(graph) {}
    ~CheckElimination() noexcept override = default;

    bool Eliminate(Graph *graph);

  private:
  private:
    bool TryRemoveCheck(SingleInstruction *instr);
    bool SingleInputCheckDominates(SingleInstruction *check,
                                   SingleInstruction *checkedValue);
    bool boundsCheckDominates(SingleInstruction *check, SingleInstruction *ref,
                              SingleInstruction *idx);
};
}; // namespace ir

#endif
#ifndef JIT_AOT_COURSE_PASS_H_
#define JIT_AOT_COURSE_PASS_H_

#include "irGen/graph.h"

namespace ir {
class OptimizationPassBase {
  public:
    explicit OptimizationPassBase(Graph *graph) : graph_(graph) {
        assert(graph_);
    }
    OptimizationPassBase(const OptimizationPassBase &) = delete;
    OptimizationPassBase &operator=(const OptimizationPassBase &) = delete;
    OptimizationPassBase(OptimizationPassBase &&) = delete;
    OptimizationPassBase &operator=(OptimizationPassBase &&) = delete;
    virtual ~OptimizationPassBase() noexcept = default;

    virtual void Run() = 0;

  protected:
    Graph *graph_;
};
} // namespace ir

#endif // JIT_AOT_COURSE_PASS_H_
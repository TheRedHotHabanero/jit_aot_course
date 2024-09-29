#ifndef JIT_AOT_COURSE_IR_GEN_IRGEN_H_
#define JIT_AOT_COURSE_IR_GEN_IRGEN_H_

#include "bb.h"
#include "graph.h"
#include <vector>

namespace ir {
class Graph;

class IRGenerator {
  public:
    IRGenerator() : graph_(nullptr) {}
    ~IRGenerator() noexcept { Clear(); }

  public:
    void CreateGraph() {
        if (graph_ == nullptr) {
            graph_ = new Graph();
        }
    }
    BB *CreateEmptyBB();
    Graph *GetGraph() const { return graph_; }
    void Clear() noexcept;

  private:
    std::vector<BB *> BBs_;
    Graph *graph_;
};

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_IRGEN_H_
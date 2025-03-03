#ifndef JIT_AOT_COURSE_IR_GEN_IRGEN_H_
#define JIT_AOT_COURSE_IR_GEN_IRGEN_H_

#include "bb.h"
#include "graph.h"
#include "domTree/arena.h"
#include <vector>
#include <cassert>

namespace ir {
class Graph;

class IRGenerator {
  public:
    IRGenerator() = delete;
    IRGenerator(ArenaAllocator *const alloc) :
          allocator_(alloc),
          BBs_(alloc->ToSTL())
    {
        assert(allocator_);
    }
    ~IRGenerator() noexcept { Clear(); }

  public:
    void CreateGraph() {
        assert(graph_ == nullptr);
        auto *instrBuilder = allocator_->template New<InstructionBuilder>(allocator_);
        graph_ = allocator_->template New<Graph>(allocator_, instrBuilder);
        return;
    }
    BB *CreateEmptyBB();
    Graph *GetGraph() { return graph_; }
    void Clear() noexcept;

  private:
    ArenaAllocator *allocator_ = nullptr;
    memory::ArenaVector<BB *> BBs_;
    Graph *graph_ = nullptr;
};

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_IRGEN_H_
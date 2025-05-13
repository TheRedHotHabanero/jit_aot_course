#ifndef JIT_AOT_COURSE_IR_GEN_COMPILER
#define JIT_AOT_COURSE_IR_GEN_COMPILER

#include "base.h"
#include "domTree/arena.h"
#include "helperBuilderFunctions.h"

namespace ir {
using namespace memory;

class Compiler : public CompilerBase {
  public:
    Compiler() : allocator_(), functionsGraphs_(allocator_.ToSTL()) {}

    Graph *CreateNewGraph() override {
        auto *instrBuilder =
            allocator_.template New<InstructionBuilder>(&allocator_);
        return CreateNewGraph(instrBuilder);
    }
    Graph *CreateNewGraph(InstructionBuilder *instrBuilder);
    Graph *CopyGraph(Graph *source, InstructionBuilder *instrBuilder) override;
    Graph *Optimize(Graph *graph) override { return graph; }
    Graph *GetFunction(FunctionID functionId) override {
        if (functionId >= functionsGraphs_.size()) {
            return nullptr;
        }
        return functionsGraphs_[functionId];
    }

    bool DeleteFunctionGraph(FunctionID functionId) override {
        if (functionId >= functionsGraphs_.size()) {
            return false;
        }
        functionsGraphs_.erase(functionsGraphs_.begin() + functionId);
        return true;
    }

  private:
    memory::ArenaAllocator allocator_;
    ArenaVector<Graph *> functionsGraphs_;
};

};     // namespace ir
#endif // JIT_AOT_COURSE_IR_GEN_COMPILER
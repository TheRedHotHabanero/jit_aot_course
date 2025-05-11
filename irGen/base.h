#ifndef JIT_AOT_COURSE_BASE_H_
#define JIT_AOT_COURSE_BASE_H_

#include "graph.h"
#include "helperBuilderFunctions.h"

namespace ir {
class CompilerBase {
  public:
    CompilerBase() = default;
    CompilerBase(const CompilerBase &) = delete;
    CompilerBase &operator=(const CompilerBase &) = delete;
    CompilerBase(CompilerBase &&) = delete;
    CompilerBase &operator=(CompilerBase &&) = delete;
    virtual ~CompilerBase() = default;

    virtual Graph *CreateNewGraph() = 0;
    virtual Graph *CopyGraph(Graph *source,
                             InstructionBuilder *instrBuilder) = 0;
    virtual Graph *Optimize(Graph *graph) = 0;
    virtual Graph *GetFunction(FunctionID functionId) = 0;
    virtual bool DeleteFunctionGraph(FunctionID functionId) = 0;
};
} // namespace ir

#endif // JIT_AOT_COURSE_BASE_H_
#ifndef JIT_AOT_COURSE_GRAPH_HELPER_H_
#define JIT_AOT_COURSE_GRAPH_HELPER_H_

#include "graph.h"
#include "helperBuilderFunctions.h"

namespace ir {
using namespace memory;

class GraphCopyHelper final {
  public:
    GraphCopyHelper(Graph *source) : source_(source) { assert(source_); }
    GraphCopyHelper(const GraphCopyHelper &) = delete;
    GraphCopyHelper &operator=(const GraphCopyHelper &) = delete;
    GraphCopyHelper(GraphCopyHelper &&) = delete;
    GraphCopyHelper &operator=(GraphCopyHelper &&) = delete;
    ~GraphCopyHelper() = default;

    Graph *CreateCopy(Graph *copyTarget);

  private:
    void Reset(Graph *copyTarget);
    void DfoCopy(BB *currentBBlock);
    void FixDFG();

  private:
    Graph *source_;
    Graph *target_;

    ArenaUnorderedMap<size_t, SingleInstruction *> *instrsTranslation_ =
        nullptr;
    ArenaUnorderedMap<size_t, BB *> *visited_ = nullptr;
};
} // namespace ir

#endif // JIT_AOT_COURSE_GRAPH_HELPER_H_
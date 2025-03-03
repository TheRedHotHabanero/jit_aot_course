#ifndef JIT_AOT_COMPILERS_DOMTREE_LOOP_CHECKER_H_
#define JIT_AOT_COMPILERS_DOMTREE_LOOP_CHECKER_H_

#include "loop.h"
#include <vector>

namespace ir {

class LoopChecker {
  public:
    void VerifyGraphLoops(Graph *graph);

  private:
    void InitializeLoopStructures(Graph *graph);
    void IdentifyBackEdges(Graph *graph);
    void OrganizeLoops();
    void ConstructLoopTree(Graph *graph);
    void DiscoverBackEdges(BB *bblock, ArenaAllocator *const allocator);
    void RecordLoopInfo(BB *header, BB *backEdgeSource,
                     ArenaAllocator *const allocator);
    void ClassifyReducibleLoop(Loop *loop);
    void CollectLoopDetails(Loop *loop, BB *bblock, DFSColors color);

    static bool IsLoopIrreducible(const BB *header, const BB *backEdgeSource) {
        return !header->Domites(backEdgeSource);
    }

  private:
    uint32_t colorCounter_ = 0;
    ArenaVector<DFSColors> *dfsColors_ = nullptr;
    size_t blockId_ = 0;
    ArenaVector<BB *> *dfsBlocks_ = nullptr;
    ArenaVector<Loop *> *loops_ = nullptr;
};

} // namespace ir

#endif // JIT_AOT_COMPILERS_DOMTREE_LOOP_CHECKER_H_

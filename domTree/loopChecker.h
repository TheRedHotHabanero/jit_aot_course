#ifndef JIT_AOT_COMPILERS_DOMTREE_LOOP_CHECKER_H_
#define JIT_AOT_COMPILERS_DOMTREE_LOOP_CHECKER_H_

#include "loop.h"
#include <vector>

namespace ir {

class LoopChecker {
  public:
    void VerifyGraphLoops(Graph *targetGraph);

  private:
    void InitializeLoopStructures(Graph *targetGraph);
    void IdentifyBackEdges();
    void OrganizeLoops();
    void ConstructLoopTree();
    void DiscoverBackEdges(BB *bblock, ArenaAllocator *const allocator);
    void RecordLoopInfo(BB *header, BB *backEdgeSource,
                        ArenaAllocator *const allocator);
    void ClassifyReducibleLoop(Loop *loop);
    void CollectLoopDetails(Loop *loop, BB *bblock);

    static bool IsLoopIrreducible(const BB *header, const BB *backEdgeSource) {
        return !header->Domites(backEdgeSource);
    }

  private:
    Graph *graph_ = nullptr;

    Marker greyMarker_;
    Marker blackMarker_;
    size_t blockId_ = 0;
    ArenaVector<BB *> *dfsBlocks_ = nullptr;
    ArenaVector<Loop *> *loops_ = nullptr;
};

} // namespace ir

#endif // JIT_AOT_COMPILERS_DOMTREE_LOOP_CHECKER_H_

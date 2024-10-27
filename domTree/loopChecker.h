#ifndef JIT_AOT_COMPILERS_DOMTREE_LOOP_CHECKER_H_
#define JIT_AOT_COMPILERS_DOMTREE_LOOP_CHECKER_H_

#include "loop.h"
#include <vector>

namespace ir {

class LoopChecker {
  public:
    void VerifyGraphLoops(Graph *graph);

  private:
    void InitializeLoopStructures(size_t bblocksCount);
    void IdentifyBackEdges(Graph *graph);
    void OrganizeLoops();
    void ConstructLoopTree(Graph *graph);
    void DiscoverBackEdges(BB *bblock);
    void RecordLoopInfo(BB *header, BB *backEdgeSource);
    void ClassifyReducibleLoop(Loop *loop);
    void CollectLoopDetails(Loop *loop, BB *bblock, DFSColors color);

    static bool IsLoopIrreducible(const BB *header, const BB *backEdgeSource) {
        return !header->Domites(backEdgeSource);
    }

  private:
    uint32_t colorCounter_;
    std::vector<DFSColors> dfsColors_;
    size_t blockId_;
    std::vector<BB *> dfsBlocks_;
    std::vector<Loop *> loops_;
};

} // namespace ir

#endif // JIT_AOT_COMPILERS_DOMTREE_LOOP_CHECKER_H_

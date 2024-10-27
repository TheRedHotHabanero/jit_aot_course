#ifndef JIT_AOT_COMPILERS_DOMTREE_LOOP_H_
#define JIT_AOT_COMPILERS_DOMTREE_LOOP_H_

#include "bb.h"
#include <algorithm>
#include <vector>

namespace ir {

enum class DFSColors : uint32_t { WHITE = 0, GREY, BLACK, COLORS_SIZE = BLACK };

class Loop {
  public:
    Loop(size_t id, BB *header, bool isIrreducible, bool isRoot = false);

    size_t GetId() const;
    BB *GetHeader();
    const BB *GetHeader() const;

    void AddBackEdge(BB *backEdgeSource);
    std::vector<BB *> GetBackEdges();
    const std::vector<BB *> &GetBackEdges() const;

    std::vector<BB *> GetBasicBlocks();
    const std::vector<BB *> &GetBasicBlocks() const;

    void AddBB(BB *bblock);

    Loop *GetOuterLoop();
    const Loop *GetOuterLoop() const;
    void SetOuterLoop(Loop *loop);

    const std::vector<Loop *> &GetInnerLoops() const;
    void AddInnerLoop(Loop *loop);

    void SetIrreducibility(bool isIrr);
    bool IsIrreducible() const;
    bool IsRoot() const;

  private:
    size_t id_;
    BB *header_;
    std::vector<BB *> backEdges_;
    std::vector<BB *> basicBlocks_;
    Loop *outerLoop_;
    std::vector<Loop *> innerLoops_;
    bool isIrreducible_;
    bool isRoot_;
};

} // namespace ir

#endif // JIT_AOT_COMPILERS_DOMTREE_LOOP_H_

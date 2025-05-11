#ifndef JIT_AOT_COMPILERS_DOMTREE_LOOP_H_
#define JIT_AOT_COMPILERS_DOMTREE_LOOP_H_

#include "arena.h"
#include "bb.h"
#include <algorithm>
#include <vector>

namespace ir {

using memory::ArenaAllocator;
using memory::ArenaVector;

enum class DFSColors : uint32_t { WHITE = 0, GREY, BLACK, COLORS_SIZE = BLACK };

class Loop {
  public:
    Loop(size_t id, BB *header, bool isIrreducible,
         ArenaAllocator *const allocator, bool isRoot = false)
        : id_(id), header_(header), backEdges_(allocator->ToSTL()),
          basicBlocks_(allocator->ToSTL()), outerLoop_(nullptr),
          innerLoops_(allocator->ToSTL()), isIrreducible_(isIrreducible),
          isRoot_(isRoot) {}

    size_t GetId() const;
    BB *GetHeader();
    const BB *GetHeader() const;

    void AddBackEdge(BB *backEdgeSource);
    ArenaVector<BB *> GetBackEdges();

    ArenaVector<BB *> GetBasicBlocks();

    void AddBB(BB *bblock);

    Loop *GetOuterLoop();
    void SetOuterLoop(Loop *loop);

    const ArenaVector<Loop *> &GetInnerLoops() const;
    void AddInnerLoop(Loop *loop);

    void SetIrreducibility(bool isIrr);
    bool IsIrreducible() const;
    bool IsRoot() const;

    void operator delete([[maybe_unused]] void *unused1,
                         [[maybe_unused]] void *unused2) noexcept {}
    void *operator new([[maybe_unused]] size_t size) = delete;
    void operator delete([[maybe_unused]] void *unused,
                         [[maybe_unused]] size_t size) {
        std::cerr << "UNREACHABLE" << std::endl;
    }
    void *operator new([[maybe_unused]] size_t size, void *ptr) noexcept {
        return ptr;
    }

  private:
    size_t id_;
    BB *header_;
    ArenaVector<BB *> backEdges_;
    ArenaVector<BB *> basicBlocks_;
    Loop *outerLoop_;
    ArenaVector<Loop *> innerLoops_;
    bool isIrreducible_;
    bool isRoot_;
};

} // namespace ir

#endif // JIT_AOT_COMPILERS_DOMTREE_LOOP_H_

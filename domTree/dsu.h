#ifndef JIT_AOT_COMPILERS_DOMTREE_DSU_H_
#define JIT_AOT_COMPILERS_DOMTREE_DSU_H_

#include "arena.h"
#include "bb.h"
#include <numeric>
#include <stdexcept>
#include <vector>

namespace ir {

using memory::ArenaVector;
class DSU {
  public:
    DSU() = delete;
    DSU(memory::ArenaVector<BB *> *labels,
        const memory::ArenaVector<size_t> *sdoms,
        ArenaAllocator *const allocator)
        : parentLinks_(labels->size(), nullptr, allocator->ToSTL()),
          labels_(labels), sdoms_(sdoms) {}
    DSU(const DSU &) = default;
    DSU &operator=(const DSU &) = default;
    DSU(DSU &&) = default;
    DSU &operator=(DSU &&) = default;
    ~DSU() = default;
    BB *Find(BB *bblock);
    size_t GetSize() const { return parentLinks_.size(); }
    void Unite(BB *target, BB *parent) {
        setParentLinks(target->GetId(), parent);
    }

    BB *getParentLinks(size_t id) { return parentLinks_.at(id); }
    void setParentLinks(size_t id, BB *bblock) { parentLinks_.at(id) = bblock; }

  private:
    void UpdateAncestorPath(BB *bblock);

  private:
    ArenaVector<BB *> parentLinks_;
    ArenaVector<BB *> *labels_;
    const ArenaVector<size_t> *sdoms_;
};

} // namespace ir
#endif // JIT_AOT_COMPILERS_DOMTREE_DSU_H_
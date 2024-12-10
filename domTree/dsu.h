#ifndef JIT_AOT_COMPILERS_DOMTREE_DSU_H_
#define JIT_AOT_COMPILERS_DOMTREE_DSU_H_

#include "bb.h"
#include <numeric>
#include <stdexcept>
#include <vector>

namespace ir {
class DSU {
  public:
    DSU() : labels_(nullptr), sdoms_(nullptr) {}
    DSU(std::vector<BB *> &labels, const std::vector<size_t> &sdoms)
        : parentLinks_(labels.size(), nullptr), labels_(&labels),
          sdoms_(&sdoms) {}
    BB *Find(BB *bblock);
    size_t GetSize() const { return parentLinks_.size(); }
    void Unite(BB *target, BB *parent) {
        parentLinks_[target->GetId()] = parent;
    }

  private:
    void UpdateAncestorPath(BB *bblock);

  private:
    std::vector<BB *> parentLinks_;
    std::vector<BB *> *labels_;
    const std::vector<size_t> *sdoms_;
};

} // namespace ir
#endif // JIT_AOT_COMPILERS_DOMTREE_DSU_H_
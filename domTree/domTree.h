#ifndef JIT_AOT_COURSE_DOMTREE_DOMTREE
#define JIT_AOT_COURSE_DOMTREE_DOMTREE

#include "arena.h"
#include "dsu.h"
#include "graph.h"
#include <cassert>
#include <vector>

/*
Algorithm description based on the reference in the README:

Step 1: Perform a depth-first search (DFS) on the input graph and assign new
labels to the vertices, corresponding to their arrival time in the DFS.
Initialize other variables required for the implementation.
//------------------------------------------------------------------------------------------------------

Step 2: Calculate the semi-dominators for all vertices using the theorem
outlined in the previous section. This computation is done vertex by vertex in
decreasing order based on their labels.

For any vertex w in the graph, there can be four different types of incoming
edges (tree edge, back edge, forward edge, and cross edge) in the spanning tree
formed by the DFS.

Maintain the forest of processed vertices using a Disjoint Set Union (DSU)
structure that supports the following operations:

Find(v): Let r be the root of the DSU tree containing node v.
If v == r, return v. Otherwise, return a node u (u != r) with the minimum
semi-dominator along the path from v to r. Union(u, v): Merge the DSU trees
containing u and v.
//-------------------------------------------------------------------------------------------------------

Step 3: Implicitly define the immediate dominator (idom) for each vertex by
applying the sixth property of semi-dominators mentioned above.
//--------------------------------------------------------------------------------------------------------

Step 4: Explicitly define the immediate dominator for each vertex by performing
the computation vertex by vertex in increasing order based on their labels.
*/

namespace ir {
class DomTreeBuilder {
  public:
    void Construct(Graph *graph);

    // Retrieves the immediate dominators for the blocks
    const ArenaVector<BB *> *GetImmediateDominators() const { return immDoms_; }

  private:
    // Initializes internal structures based on the number of basic blocks
    DSU InitializeStructures(Graph *graph);

    // Analyzes the specified basic block using DFS
    void PerformDFS(BB *block);

    // Calculates semi-dominators for all blocks
    void DeriveSemiDominators(DSU &sdomsHelper);

    // Calculates immediate dominators for all blocks
    void DeriveImmediateDominators();

    size_t getSize() const { return immDoms_->size(); }

    BB *getImmDominator(size_t id) { return immDoms_->at(id); }
    void setImmDominator(size_t id, BB *bblock) { immDoms_->at(id) = bblock; }

    size_t getSemiDomNumber(BB *bblock) {
        assert(bblock);
        return semiDoms_->at(bblock->GetId());
    }
    void setSemiDomNumber(BB *bblock, size_t visitNumber) {
        assert(bblock);
        semiDoms_->at(bblock->GetId()) = visitNumber;
    }

    const ArenaVector<BB *> &getSemiDoms(BB *bblock) {
        assert(bblock);
        return semiDomSet_->at(bblock->GetId());
    }
    void registerSemiDom(BB *bblock) {
        auto semiDomNumber = getSemiDomNumber(bblock);
        semiDomSet_->at(getOrderedBlock(semiDomNumber)->GetId())
            .push_back(bblock);
    }

    BB *getLabel(BB *bblock) {
        assert(bblock);
        return nodeLabels_->at(bblock->GetId());
    }

    BB *getOrderedBlock(size_t visitNumber) {
        return orderedBlocks_->at(visitNumber);
    }
    void setOrderedBlock(size_t visitNumber, BB *bblock) {
        assert(bblock);
        orderedBlocks_->at(visitNumber) = bblock;
    }

    BB *getBlockDFOParent(BB *bblock) {
        assert(bblock);
        return blockAncestors_->at(bblock->GetId());
    }
    void setBlockDFOParent(BB *child, BB *parent) {
        assert(child);
        assert(parent);
        blockAncestors_->at(child->GetId()) = parent;
    }

    // Member variables
    int lastVisited_ = 0; // Last visited vertex count during DFS
    ArenaVector<BB *> *immDoms_ = nullptr;    // List of imm doms
    ArenaVector<size_t> *semiDoms_ = nullptr; // Semi-doms for each block
    ArenaVector<ArenaVector<BB *>> *semiDomSet_ =
        nullptr;                              // Sets of semi-dominators
    ArenaVector<BB *> *nodeLabels_ = nullptr; // Labels assigned during DFS
    ArenaVector<BB *> *orderedBlocks_ =
        nullptr; // Blocks ordered by their DFS visitation
    ArenaVector<BB *> *blockAncestors_ =
        nullptr; // Ancestors of each block in the DFS tree
};
} // namespace ir

#endif // JIT_AOT_COURSE_DOMTREE_DOMTREE

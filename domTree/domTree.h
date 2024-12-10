#ifndef JIT_AOT_COURSE_DOMTREE_DOMTREE
#define JIT_AOT_COURSE_DOMTREE_DOMTREE

#include "dsu.h"
#include "graph.h"
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
    const std::vector<BB *> &GetImmediateDominators() const { return immDoms_; }

  private:
    // Initializes internal structures based on the number of basic blocks
    void InitializeStructures(size_t blockCount);

    // Analyzes the specified basic block using DFS
    void PerformDFS(BB *block);

    // Calculates semi-dominators for all blocks
    void DeriveSemiDominators();

    // Calculates immediate dominators for all blocks
    void DeriveImmediateDominators();

    // Member variables
    int lastVisited_;              // Last visited vertex count during DFS
    DSU semiDomHelper_;            // DSU for semi-doms
    std::vector<BB *> immDoms_;    // List of imm doms
    std::vector<size_t> semiDoms_; // Semi-doms for each block
    std::vector<std::vector<BB *>> semiDomSet_; // Sets of semi-dominators
    std::vector<BB *> nodeLabels_;              // Labels assigned during DFS
    std::vector<BB *> orderedBlocks_; // Blocks ordered by their DFS visitation
    std::vector<BB *>
        blockAncestors_; // Ancestors of each block in the DFS tree
};
} // namespace ir

#endif // JIT_AOT_COURSE_DOMTREE_DOMTREE

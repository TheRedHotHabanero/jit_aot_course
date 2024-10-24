#ifndef JIT_AOT_COURSE_DOMTREE_DFO_RPO
#define JIT_AOT_COURSE_DOMTREE_DFO_RPO

#include "bb.h"
#include "graph.h"
#include <functional>
#include <iostream>
#include <set>
#include <vector>

namespace ir {

using DFOCallback = std::function<void(BB *)>;

class DFO {
  public:
    ~DFO() = default;

    void ValidateGraph(Graph *graph, DFOCallback callback) {
        if (!IsGraphValid(graph)) {
            return;
        }

        ResetVisited(); // Clear the visited set
        ExecuteDFS(graph->GetFirstBB(), callback);

        // Verify the number of visited blocks
        if (visited_.size() != graph->GetBBCount()) {
            HandleError(
                "Visited blocks count does not match graph's block count.");
        }
    }

    // Validate the graph pointer
    bool IsGraphValid(Graph *graph) {
        if (!graph) {
            HandleError("Graph pointer is null.");
            return false;
        }
        return !graph->IsEmpty();
    }

  private:
    std::set<size_t> visited_; // Set of visited basic blocks

    // Reset the visited set for a new traversal
    void ResetVisited() { visited_.clear(); }

    // Recursive function to perform depth-first search on the basic blocks
    void ExecuteDFS(BB *bblock, DFOCallback callback) {
        if (!IsBlockValid(bblock))
            return; // Validate the block

        visited_.insert(bblock->GetId()); // Mark the block as visited

        // Visit all successors of the current block
        for (auto *successor : bblock->GetSuccessors()) {
            if (!HasBeenVisited(successor)) {
                ExecuteDFS(successor, callback); // Visit unvisited successor
            }
        }

        // Invoke the callback for the current block
        callback(bblock);
    }

    // Validate the basic block pointer
    bool IsBlockValid(BB *bblock) {
        if (!bblock) {
            HandleError("Basic block pointer is null.");
            return false;
        }
        return true;
    }

    // Check if a block has been visited
    bool HasBeenVisited(BB *bblock) {
        return visited_.find(bblock->GetId()) != visited_.end();
    }

    // Handle errors by printing the message and aborting
    void HandleError(const std::string &message) {
        std::cerr << "[DFO&PRO Error] " << message << std::endl;
        std::abort();
    }
};

// Function to get the Reverse Post Order (RPO) of the graph
std::vector<BB *> RPO(Graph *graph) {
    DFO dfo;
    if (!dfo.IsGraphValid(graph))
        return {}; // Check graph validity and return empty if invalid

    std::vector<BB *> result;            // Vector to hold the RPO
    result.reserve(graph->GetBBCount()); // Reserve space for the result

    dfo.ValidateGraph(graph, [&result](BB *bblock) {
        result.push_back(bblock); // Collect blocks in DFO
    });

    std::reverse(result.begin(), result.end()); // Reverse the order to get RPO
    return result;                              // Return the computed RPO
}

} // namespace ir

#endif // JIT_AOT_COURSE_DOMTREE_DFO_RPO

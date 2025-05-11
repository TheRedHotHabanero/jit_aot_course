#ifndef JIT_AOT_COURSE_DOMTREE_DFO_RPO
#define JIT_AOT_COURSE_DOMTREE_DFO_RPO

#include "arena.h"
#include "bb.h"
#include "graph.h"
#include <functional>
#include <iostream>
#include <set>
#include <vector>

namespace ir {

using DFOCallback = std::function<void(BB *)>;
using memory::ArenaSet;
using memory::ArenaVector;

class DFO {
  public:
    ~DFO() = default;

    template <typename GraphT, typename CallbackT>
    void ValidateGraph(GraphT *graph, CallbackT callback) {
        assert(graph);
        if (graph->IsEmpty()) {
            return;
        }

        if (!visited_) {
            auto *allocator = graph->GetAllocator();
            visited_ = allocator->template New<memory::ArenaSet<size_t>>(
                allocator->ToSTL());
        } else {
            visited_->clear();
        }
        ExecuteDFS(graph->GetFirstBB(), callback);

        // Verify the number of visited blocks
        if (visited_->size() != graph->GetBBCount()) {
            HandleError(
                "Visited blocks count does not match graph's block count.");
        }
    }

    static void Run(Graph *graph, DFOCallback callback) {
        auto dfo = DFO();
        dfo.ValidateGraph(graph, callback);
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
    memory::ArenaSet<size_t> *visited_ = nullptr; // Set of visited basic blocks

    // Reset the visited set for a new traversal
    void ResetVisited() { visited_->clear(); }

    // Recursive function to perform depth-first search on the basic blocks
    template <typename BBlockT, typename CallbackT>
    void ExecuteDFS(BBlockT *bblock, CallbackT callback) {
        assert(bblock);
        visited_->insert(bblock->GetId());
        for (auto *succ : bblock->GetSuccessors()) {
            if (!visited_->contains(succ->GetId())) {
                ExecuteDFS(succ, callback);
            }
        }
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
        return visited_->find(bblock->GetId()) != visited_->end();
    }

    // Handle errors by printing the message and aborting
    void HandleError(const std::string &message) {
        std::cerr << "[DFO&PRO Error] " << message << std::endl;
        std::abort();
    }
};

template <typename GraphT> struct BasicBlockTypeHelper {
    using type = const BB;
};

template <> struct BasicBlockTypeHelper<Graph> { using type = BB; };

template <typename GraphT>
using BasickBlockType = typename BasicBlockTypeHelper<GraphT>::type;

template <typename GraphT>
memory::ArenaVector<BasickBlockType<GraphT> *> RPO(GraphT *graph) {
    assert(graph);
    memory::ArenaVector<BasickBlockType<GraphT> *> result(
        graph->GetAllocator()->ToSTL());
    if (graph->IsEmpty()) {
        return result;
    }

    result.reserve(graph->GetBBCount());
    DFO::Run(graph, [&result](BasickBlockType<GraphT> *bblock) {
        result.push_back(bblock);
    });
    assert(result.size() == graph->GetBBCount());
    std::reverse(result.begin(), result.end());
    return result;
}

// Function to get the Reverse Post Order (RPO) of the graph
template <typename GraphT> inline ArenaVector<BB *> RPO(Graph *graph) {
    assert(graph);
    memory::ArenaVector<BasickBlockType<GraphT> *> result(
        graph->GetAllocator()->ToSTL());
    if (graph->IsEmpty()) {
        return result;
    }

    result.reserve(graph->GetBBCount());
    DFO::Run(graph, [&result](BasickBlockType<GraphT> *bblock) {
        result.push_back(bblock);
    });
    assert(result.size() == graph->GetBBCount());
    std::reverse(result.begin(), result.end());
    return result; // Return the computed RPO
}

inline void DumpBB(BB *bblock) {
    assert(bblock);
    std::cout << "=== BB #" << bblock->GetId() << " (" << bblock->GetSize();
    std::cout << " instructions)\npreds: < ";
    for (const auto &pred : bblock->GetPredecessors()) {
        std::cout << pred->GetId() << " ";
    }
    std::cout << ">\nsuccs: < ";
    for (const auto &succ : bblock->GetSuccessors()) {
        std::cout << succ->GetId() << " ";
    }
    std::cout << ">\n";
}

inline void DumpGraphRPO(Graph *graph) {
    auto rpoBBlocks = RPO(graph);
    std::cout << "======================================\n";
    for (auto bblock : rpoBBlocks) {
        DumpBB(bblock);
    }
    std::cout << "======================================" << std::endl;
}

inline void DumpGraphDFO(Graph *graph) {
    std::cout << "======================================\n";
    DFO::Run(graph, DumpBB);
    std::cout << "======================================" << std::endl;
}

} // namespace ir

#endif // JIT_AOT_COURSE_DOMTREE_DFO_RPO

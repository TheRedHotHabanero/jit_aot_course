#include "domTree.h"
#include <numeric>

namespace ir {

void DomTreeBuilder::Construct(Graph *graph) {
    if (!graph) {
        std::cerr << "[DomTreeBuilder Error] Invalid Graph in Construct"
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Terminate early for empty graphs
    if (graph->IsEmpty()) {
        return;
    }

    // Prepare internal structures for processing
    InitializeStructures(graph->GetBBCount());

    // Begin depth-first search from the first basic block
    PerformDFS(graph->GetFirstBB());

    // Validate the graph's connectivity
    if (lastVisited_ != static_cast<int>(graph->GetBBCount()) - 1) {
        std::cerr << "[DomTreeBuilder Error] Graph is not fully connected"
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Calculate semi-dominators for all blocks
    DeriveSemiDominators();

    // Calculate immediate dominators
    DeriveImmediateDominators();
}

void DomTreeBuilder::InitializeStructures(size_t blockCount) {
    // Resetting internal variables for the new graph
    lastVisited_ = -1;
    semiDoms_.resize(blockCount, INVALID_BB_ID);
    semiDomSet_.clear();
    semiDomSet_.resize(blockCount);
    immDoms_.resize(blockCount, nullptr);
    nodeLabels_.resize(blockCount, nullptr);
    orderedBlocks_.resize(blockCount, nullptr);
    blockAncestors_.resize(blockCount, nullptr);

    // DSU initializing for semi-dominators
    semiDomHelper_ = DSU(nodeLabels_, semiDoms_);
}

void DomTreeBuilder::PerformDFS(BB *block) {
    ++lastVisited_;

    if (!block || lastVisited_ >= static_cast<int>(orderedBlocks_.size())) {
        std::cerr << "[DomTreeBuilder Error] Invalid Block in PerformDFS"
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Set the current block's label and semi-dominator
    auto blockId = block->GetId();
    nodeLabels_[blockId] = block;
    semiDoms_[blockId] = lastVisited_;
    orderedBlocks_[lastVisited_] = block;

    // Recursively analyze each successor block
    for (auto *successor : block->GetSuccessors()) {
        if (!nodeLabels_[successor->GetId()]) {
            blockAncestors_[successor->GetId()] = block; // Record ancestor
            PerformDFS(successor); // Continue DFS on successor
        }
    }
}

void DomTreeBuilder::DeriveSemiDominators() {
    // Process blocks in reverse order of DFS
    for (int i = static_cast<int>(orderedBlocks_.size()) - 1; i >= 0; --i) {
        auto *currentBlock = orderedBlocks_[i];
        auto currentId = currentBlock->GetId();

        // UPD semi-doms using predecessors
        for (const auto &pred : currentBlock->GetPredecessors()) {
            auto minLabelNode = semiDomHelper_.Find(pred);
            semiDoms_[currentId] = std::min(semiDoms_[currentId],
                                            semiDoms_[minLabelNode->GetId()]);
        }

        // Unite the current block with its ancestor in DSU
        if (i > 0) {
            semiDomSet_[orderedBlocks_[semiDoms_[currentId]]->GetId()]
                .push_back(currentBlock);
            auto *parentBlock = blockAncestors_[currentId];
            semiDomHelper_.Unite(currentBlock, parentBlock);
        }

        // Identify immediate dominators from semi-dominators
        for (auto dominatee : semiDomSet_[currentId]) {
            auto minSemiDom = semiDomHelper_.Find(dominatee);
            auto dominateeId = dominatee->GetId();
            if (semiDoms_[minSemiDom->GetId()] == semiDoms_[dominateeId]) {
                immDoms_[dominateeId] = orderedBlocks_[semiDoms_[dominateeId]];
            } else {
                immDoms_[dominateeId] = minSemiDom;
            }
        }
    }
}

void DomTreeBuilder::DeriveImmediateDominators() {
    // Refine immediate dominators iteratively
    for (size_t i = 1; i < orderedBlocks_.size(); ++i) {
        auto *currentBlock = orderedBlocks_[i];
        auto currentId = currentBlock->GetId();

        // Update the dominator if necessary
        if (immDoms_[currentId]->GetId() !=
            orderedBlocks_[semiDoms_[currentId]]->GetId()) {
            immDoms_[currentId] = immDoms_[immDoms_[currentId]->GetId()];
        }

        // Assign the current block's dominator
        currentBlock->SetDominator(immDoms_[currentId]);

        // Add the current block to its dominator's list of dominated blocks
        immDoms_[currentId]->AddDominatedBlock(currentBlock);
    }
}
} // namespace ir

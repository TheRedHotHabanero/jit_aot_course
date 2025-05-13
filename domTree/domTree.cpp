#include "domTree.h"
#include <numeric>

namespace ir {

void DomTreeBuilder::Construct(Graph *graph) {
    assert(graph);

    // Terminate early for empty graphs
    if (graph->IsEmpty()) {
        return;
    }

    // Prepare internal structures for processing
    auto sdomsHelper = InitializeStructures(graph);
    // Begin depth-first search from the first basic block
    PerformDFS(graph->GetFirstBB());

    // Validate the graph's connectivity
    assert(lastVisited_ == static_cast<int>(graph->GetBBCount()) - 1);

    // Calculate semi-dominators for all blocks
    DeriveSemiDominators(sdomsHelper);

    // Calculate immediate dominators
    DeriveImmediateDominators();
}

DSU DomTreeBuilder::InitializeStructures(Graph *graph) {
    lastVisited_ = -1;
    // Resetting internal variables for the new graph
    auto bblocksCount = graph->GetBBCount();
    auto *allocator = graph->GetAllocator();
    if (semiDoms_ == nullptr) {
        semiDoms_ = allocator->template NewVector<size_t>(bblocksCount,
                                                          ir::INVALID_BB_ID);
        semiDomSet_ = allocator->template NewVector<ArenaVector<BB *>>(
            bblocksCount, ArenaVector<BB *>(allocator->ToSTL()));
        immDoms_ = allocator->template NewVector<BB *>(bblocksCount, nullptr);
        nodeLabels_ =
            allocator->template NewVector<BB *>(bblocksCount, nullptr);
        orderedBlocks_ =
            allocator->template NewVector<BB *>(bblocksCount, nullptr);
        blockAncestors_ =
            allocator->template NewVector<BB *>(bblocksCount, nullptr);
    } else {
        semiDoms_->clear();
        semiDomSet_->clear();
        immDoms_->clear();
        nodeLabels_->clear();
        orderedBlocks_->clear();
        blockAncestors_->clear();

        semiDoms_->resize(bblocksCount, ir::INVALID_BB_ID);
        semiDomSet_->resize(bblocksCount,
                            ArenaVector<BB *>(allocator->ToSTL()));
        immDoms_->resize(bblocksCount, nullptr);
        nodeLabels_->resize(bblocksCount, nullptr);
        orderedBlocks_->resize(bblocksCount, nullptr);
        blockAncestors_->resize(bblocksCount, nullptr);
    }
    return DSU(nodeLabels_, semiDoms_, allocator);
}

void DomTreeBuilder::PerformDFS(BB *block) {
    ++lastVisited_;
    assert((block) && (lastVisited_ < static_cast<int>(getSize())));

    // Set the current block's label and semi-dominator
    auto blockId = block->GetId();
    nodeLabels_->at(blockId) = block;
    setSemiDomNumber(block, lastVisited_);
    setOrderedBlock(lastVisited_, block);

    // Recursively analyze each successor block
    for (auto *successor : block->GetSuccessors()) {
        if (getLabel(successor) == nullptr) {
            setBlockDFOParent(successor, block); // Record ancestor
            PerformDFS(successor);               // Continue DFS on successor
        }
    }
}

void DomTreeBuilder::DeriveSemiDominators(DSU &sdomsHelper) {
    // Process blocks in reverse order of DFS

    for (int i = getSize() - 1; i >= 0; --i) {
        auto *currentBlock = getOrderedBlock(i);
        assert(currentBlock);
        for (const auto &pred : currentBlock->GetPredecessors()) {
            auto nodeWithMinLabel = sdomsHelper.Find(pred);
            auto id = std::min(getSemiDomNumber(currentBlock),
                               getSemiDomNumber(nodeWithMinLabel));
            setSemiDomNumber(currentBlock, id);
        }

        if (i > 0) {
            registerSemiDom(currentBlock);
            sdomsHelper.Unite(currentBlock, getBlockDFOParent(currentBlock));
        }

        for (auto dominatee : getSemiDoms(currentBlock)) {
            auto minSDom = sdomsHelper.Find(dominatee);

            auto dominateeId = dominatee->GetId();
            if (getSemiDomNumber(minSDom) == getSemiDomNumber(dominatee)) {
                setImmDominator(dominateeId,
                                getOrderedBlock(getSemiDomNumber(dominatee)));
            } else {
                setImmDominator(dominateeId, minSDom);
            }
        }
    }
}

void DomTreeBuilder::DeriveImmediateDominators() {
    // Refine immediate dominators iteratively

    for (size_t i = 1; i < getSize(); ++i) {
        auto *currentBlock = getOrderedBlock(i);
        auto currentBlockId = currentBlock->GetId();
        if (getImmDominator(currentBlockId) !=
            getOrderedBlock(getSemiDomNumber(currentBlock))) {
            setImmDominator(
                currentBlockId,
                getImmDominator(getImmDominator(currentBlockId)->GetId()));
        }

        auto *immDom = getImmDominator(currentBlockId);
        currentBlock->SetDominator(immDom);
        immDom->AddDominatedBlock(currentBlock);
    }
}
} // namespace ir

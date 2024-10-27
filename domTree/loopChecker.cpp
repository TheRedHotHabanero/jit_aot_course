#include "loopChecker.h"
#include "domTree.h"
#include <iostream>

namespace ir {

void LoopChecker::VerifyGraphLoops(Graph *graph) {
    if (!graph) {
        std::cerr << "[Loop Checker Error] Graph is null" << std::endl;
        std::abort();
    }
    if (graph->IsEmpty()) {
        return;
    }

    InitializeLoopStructures(graph->GetBBCount());
    DomTreeBuilder().Construct(graph);
    IdentifyBackEdges(graph);
    OrganizeLoops();
    ConstructLoopTree(graph);
}

void LoopChecker::InitializeLoopStructures(size_t bblocksCount) {
    colorCounter_ = static_cast<uint32_t>(DFSColors::COLORS_SIZE);
    dfsColors_.assign(bblocksCount, DFSColors::WHITE);
    blockId_ = 0;
    dfsBlocks_.assign(bblocksCount, nullptr);
    loops_.clear();
}

void LoopChecker::IdentifyBackEdges(Graph *graph) {
    if (!graph) {
        std::cerr << "[LoopChecker Error] Graph is null while identifying back edges" << std::endl;
        std::abort();
    }

    for (BB *bblock : graph->GetBBs()) {
        if (dfsColors_[bblock->GetId()] == DFSColors::WHITE) {
            DiscoverBackEdges(bblock);
        }
    }

    if (blockId_ != graph->GetBBCount()) {
        std::cerr << "[LoopChecker Error] Block count mismatch" << std::endl;
        std::abort();
    }
}

void LoopChecker::OrganizeLoops() {
    for (auto *bblock : dfsBlocks_) {
        auto *loop = bblock->GetLoop();
        if (!loop || loop->GetHeader() != bblock) {
            continue;
        }
        if (loop->IsIrreducible()) {
            for (auto *backEdgeSource : loop->GetBackEdges()) {
                if (!backEdgeSource->GetLoop()) {
                    backEdgeSource->SetLoop(loop);
                }
            }
        } else {
            ClassifyReducibleLoop(loop);
        }
    }
}

void LoopChecker::ConstructLoopTree(Graph *graph) {
    if (!graph) {
        std::cerr << "[LoopChecker Error] Graph is null while constructing loop tree" << std::endl;
        std::abort();
    }

    auto *rootLoop = new Loop(loops_.size(), nullptr, false, true);
    loops_.push_back(rootLoop);

    for (auto *bblock : dfsBlocks_) {
        auto *loop = bblock->GetLoop();
        if (!loop) {
            rootLoop->AddBB(bblock);
        } else if (!loop->GetOuterLoop()) {
            loop->SetOuterLoop(rootLoop);
            rootLoop->AddInnerLoop(loop);
        }
    }

    graph->SetLoopTree(rootLoop);
}

void LoopChecker::DiscoverBackEdges(BB *bblock) {
    if (!bblock) {
        std::cerr << "[LoopChecker Error] Block is null" << std::endl;
        std::abort();
    }

    dfsColors_[bblock->GetId()] = DFSColors::GREY;
    for (BB *succ : bblock->GetSuccessors()) {
        auto &color = dfsColors_[succ->GetId()];
        if (color == DFSColors::WHITE) {
            DiscoverBackEdges(succ);
        } else if (color == DFSColors::GREY) {
            RecordLoopInfo(succ, bblock);
        }
    }

    dfsColors_[bblock->GetId()] = DFSColors::BLACK;
    dfsBlocks_[blockId_++] = bblock;
}

void LoopChecker::RecordLoopInfo(BB *header, BB *backEdgeSource) {
    Loop *loop = header->GetLoop();
    if (!loop) {
        loop = new Loop(loops_.size(), header, IsLoopIrreducible(header, backEdgeSource));
        loop->AddBB(header);
        loop->AddBackEdge(backEdgeSource);
        loops_.push_back(loop);
        header->SetLoop(loop);
    } else {
        loop->AddBackEdge(backEdgeSource);
        if (IsLoopIrreducible(header, backEdgeSource)) {
            loop->SetIrreducibility(true);
        }
    }
}

void LoopChecker::ClassifyReducibleLoop(Loop *loop) {
    if (!loop) {
        std::cerr << "[Loop Checker Error] Loop is null during reducible loop classification" << std::endl;
        std::abort();
    }

    auto color = static_cast<DFSColors>(++colorCounter_);
    dfsColors_[loop->GetHeader()->GetId()] = color;

    for (BB *backEdgeSource : loop->GetBackEdges()) {
        CollectLoopDetails(loop, backEdgeSource, color);
    }
}

void LoopChecker::CollectLoopDetails(Loop *loop, BB *bblock, DFSColors color) {
    if (!loop || !bblock) {
        std::cerr << "[LoopChecker Error] Loop or block is null" << std::endl;
        std::abort();
    }

    if (dfsColors_[bblock->GetId()] == color) {
        return; // Already processed
    }

    dfsColors_[bblock->GetId()] = color;
    Loop *blockLoop = bblock->GetLoop();

    if (!blockLoop) {
        loop->AddBB(bblock);
    } else if (blockLoop->GetId() != loop->GetId() && !blockLoop->GetOuterLoop()) {
        blockLoop->SetOuterLoop(loop);
        loop->AddInnerLoop(blockLoop);
    }

    for (BB *pred : bblock->GetPredecessors()) {
        CollectLoopDetails(loop, pred, color);
    }
}

} // namespace ir

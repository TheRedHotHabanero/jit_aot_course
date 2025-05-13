#include "loopChecker.h"
#include "domTree.h"
#include <iostream>

namespace ir {

void LoopChecker::VerifyGraphLoops(Graph *targetGraph) {
    if (!targetGraph) {
        std::cerr << "[Loop Checker Error] Graph is null" << std::endl;
        std::abort();
    }
    if (targetGraph->IsEmpty()) {
        return;
    }

    InitializeLoopStructures(targetGraph);
    DomTreeBuilder().Construct(targetGraph);
    IdentifyBackEdges();
    targetGraph->ReleaseMarker(greyMarker_);
    targetGraph->ReleaseMarker(blackMarker_);
    OrganizeLoops();
    ConstructLoopTree();
}

void LoopChecker::InitializeLoopStructures(Graph *targetGraph) {
    graph_ = targetGraph;
    blockId_ = 0;
    auto bblocksCount = graph_->GetBBCount();
    if (dfsBlocks_ == nullptr) {
        auto *allocator = graph_->GetAllocator();
        dfsBlocks_ = allocator->NewVector<BB *>(bblocksCount, nullptr);
        loops_ = allocator->NewVector<Loop *>();
    } else {
        dfsBlocks_->clear();
        dfsBlocks_->clear();
        loops_->clear();

        dfsBlocks_->resize(bblocksCount, nullptr);
    }
    greyMarker_ = graph_->GetNewMarker();
    blackMarker_ = graph_->GetNewMarker();
}

void LoopChecker::IdentifyBackEdges() {
    assert(graph_);
    DiscoverBackEdges(graph_->GetFirstBB(), graph_->GetAllocator());
    assert(blockId_ == graph_->GetBBCount());
}

void LoopChecker::OrganizeLoops() {
    for (auto *bblock : *dfsBlocks_) {
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

void LoopChecker::ConstructLoopTree() {
    if (!graph_) {
        std::cerr
            << "[LoopChecker Error] Graph is null while constructing loop tree"
            << std::endl;
        std::abort();
    }

    auto *allocator = graph_->GetAllocator();
    auto *rootLoop = allocator->template New<Loop>(loops_->size(), nullptr,
                                                   false, allocator, true);
    loops_->push_back(rootLoop);

    for (auto *bblock : *dfsBlocks_) {
        auto *loop = bblock->GetLoop();
        if (!loop) {
            rootLoop->AddBB(bblock);
        } else if (!loop->GetOuterLoop()) {
            loop->SetOuterLoop(rootLoop);
            rootLoop->AddInnerLoop(loop);
        }
    }

    graph_->SetLoopTree(rootLoop);
}

void LoopChecker::DiscoverBackEdges(BB *bblock,
                                    ArenaAllocator *const allocator) {
    if (!bblock) {
        std::cerr << "[LoopChecker Error] Block is null" << std::endl;
        std::abort();
    }
    bblock->SetMarker(greyMarker_);
    for (BB *succ : bblock->GetSuccessors()) {
        if (succ->IsMarkerSet(greyMarker_)) {
            RecordLoopInfo(succ, bblock, allocator);
        } else if (!succ->IsMarkerSet(blackMarker_)) {
            DiscoverBackEdges(succ, allocator);
        }
    }

    bblock->ClearMarker(greyMarker_);
    bblock->SetMarker(blackMarker_);
    dfsBlocks_->at(blockId_++) = bblock;
}

void LoopChecker::RecordLoopInfo(BB *header, BB *backEdgeSource,
                                 ArenaAllocator *const allocator) {
    Loop *loop = header->GetLoop();
    if (!loop) {
        loop = allocator->template New<Loop>(
            loops_->size(), header, IsLoopIrreducible(header, backEdgeSource),
            allocator);
        loop->AddBB(header);
        loop->AddBackEdge(backEdgeSource);
        loops_->push_back(loop);
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
        std::cerr << "[Loop Checker Error] Loop is null during reducible loop "
                     "classification"
                  << std::endl;
        std::abort();
    }

    blackMarker_ = graph_->GetNewMarker();
    loop->GetHeader()->SetMarker(blackMarker_);

    for (BB *backEdgeSource : loop->GetBackEdges()) {
        CollectLoopDetails(loop, backEdgeSource);
    }
    graph_->ReleaseMarker(blackMarker_);
}

void LoopChecker::CollectLoopDetails(Loop *loop, BB *bblock) {
    if (!loop || !bblock) {
        std::cerr << "[LoopChecker Error] Loop or block is null" << std::endl;
        std::abort();
    }

    if (bblock->IsMarkerSet(blackMarker_)) {
        return; // Already processed
    }

    bblock->SetMarker(blackMarker_);
    Loop *blockLoop = bblock->GetLoop();

    if (!blockLoop) {
        loop->AddBB(bblock);
    } else if (blockLoop->GetId() != loop->GetId() &&
               !blockLoop->GetOuterLoop()) {
        blockLoop->SetOuterLoop(loop);
        loop->AddInnerLoop(blockLoop);
    }

    for (BB *pred : bblock->GetPredecessors()) {
        CollectLoopDetails(loop, pred);
    }
}

} // namespace ir

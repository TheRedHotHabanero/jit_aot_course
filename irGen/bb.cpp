#include "bb.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <vector>
namespace ir {

// const size_t BB::INVALID_BB_ID;

void BB::AddSuccessors(BB *bb) {
    if (successors_.size() > 2) {
        std::cout << "[BB Error] More than 2 successors!" << std::endl;
        std::abort();
    }
    if (bb == nullptr) {
        std::cout << "[BB Error] Nullptr Basic Block! (AddSuccessors)"
                  << std::endl;
        std::abort();
    }
    successors_.push_back(bb);
}

void BB::DeleteSuccessors(BB *bb) {
    if (bb == nullptr) {
        std::cout << "[BB Error] Nullptr Basic Block! (DeleteSuccessors)"
                  << std::endl;
        std::abort();
    }
    auto it = std::find(successors_.begin(), successors_.end(), bb);
    if (it != successors_.end()) {
        successors_.erase(it);
    } else {
        std::cout << "[BB Error] DeleteSuccessors." << std::endl;
        std::abort();
    }
}

void BB::AddPredecessors(BB *bb) {
    if (bb == nullptr) {
        std::cout << "[BB Error] Nullptr Basic Block! (AddPredecessors)"
                  << std::endl;
        std::abort();
    }
    predecessors_.push_back(bb);
}

void BB::DeletePredecessors(BB *bb) {
    if (bb == nullptr) {
        std::cout << "[BB Error] Nullptr Basic Block! (DeletePredecessors)"
                  << std::endl;
        std::abort();
    }
    auto it = std::find(predecessors_.begin(), predecessors_.end(), bb);
    if (it != predecessors_.end()) {
        *it = predecessors_.back();
        predecessors_.pop_back();
    } else {
        std::cout << "[BB Error] DeletePredecessors." << std::endl;
        std::abort();
    }
}

void BB::SetInstructionAsDead(SingleInstruction *inst) {
    if (inst == nullptr || inst->GetInstBB() != this) {
        std::cout << "[BB Error] Error while trying set single inst as dead."
                  << std::endl;
        std::abort();
    }
    inst->SetBB(nullptr); // unlink
    auto *tmpPrev = inst->GetPrevInst();
    auto *tmpNext = inst->GetNextInst();
    inst->SetPrevInst(nullptr);
    inst->SetNextInst(nullptr);

    if (tmpPrev) {
        tmpPrev->SetNextInst(tmpNext);
    }

    if (tmpNext) {
        tmpNext->SetPrevInst(tmpPrev);
    }

    if (inst->IsPhi()) {
        if (inst == firstPhiBB_) {
            if (inst == lastPhiBB_) {
                firstPhiBB_ = nullptr;
                lastPhiBB_ = nullptr;
            } else {
                assert((tmpNext) && tmpNext->IsPhi());
                firstPhiBB_ = static_cast<PhiInstr *>(tmpNext);
            }
        } else if (inst == lastPhiBB_) {
            assert((tmpPrev) && tmpPrev->IsPhi());
            lastPhiBB_ = static_cast<PhiInstr *>(tmpPrev);
        }
    } else {
        if (inst == firstInstBB_) {
            firstInstBB_ = tmpNext;
        }
        if (inst == lastInstBB_) {
            lastInstBB_ = (tmpPrev && tmpPrev->IsPhi()) ? nullptr : tmpPrev;
        }
    }
    size_ -= 1;
}

void BB::ReplaceInstruction(SingleInstruction *prevInstr,
                            SingleInstruction *newInstr) {
    prevInstr->ReplaceInputInUsers(newInstr);
    ReplaceInControlFlow(prevInstr, newInstr);
}

void BB::ReplaceSuccessor(BB *prevSucc, BB *newSucc) {
    assert((prevSucc) && (newSucc));
    auto it = std::find(successors_.begin(), successors_.end(), prevSucc);
    assert(it != successors_.end());
    *it = newSucc;
}

void BB::ReplaceInControlFlow(SingleInstruction *prevInstr,
                              SingleInstruction *newInstr) {
    assert((prevInstr) && (prevInstr->GetInstBB() == this));
    if (prevInstr->GetPrevInst()) {
        InsertSingleInstrAfter(prevInstr->GetPrevInst(), newInstr);
    } else if (prevInstr->GetNextInst()) {
        InsertSingleInstrBefore(prevInstr->GetNextInst(), newInstr);
    } else {
        PushInstForward(newInstr);
    }
    SetInstructionAsDead(prevInstr);
}

void BB::InsertSingleInstrBefore(SingleInstruction *instToMove,
                                 SingleInstruction *currentInstr) {
    // PrevInst  →  instToMove  →  NextInst
    // PrevInst  →  CurrentInstr  →  instToMove  →  NextInst
    if (instToMove == nullptr || currentInstr == nullptr) {
        std::cout << "[BB Error] One of instructions went nullptr "
                     "(InsertSingleInstrBefore)"
                  << std::endl;
        std::abort();
    }
    if (instToMove->GetInstBB() == nullptr ||
        currentInstr->GetInstBB() == nullptr) {
        std::cout
            << "[BB Error] One of BB went nullptr (InsertSingleInstrBefore)"
            << std::endl;
        std::abort();
    }
    currentInstr->SetBB(this);
    auto *tmpPrev = instToMove->GetPrevInst();
    instToMove->SetPrevInst(currentInstr);
    currentInstr->SetPrevInst(tmpPrev);
    currentInstr->SetNextInst(instToMove);
    if (tmpPrev) {
        tmpPrev->SetNextInst(currentInstr);
    }
    // check branches

    if (!tmpPrev) {
        firstInstBB_ = currentInstr;
    }
    size_ += 1;
}

void BB::InsertSingleInstrAfter(SingleInstruction *instToInsert,
                                SingleInstruction *currentInstr) {
    if (instToInsert == nullptr || currentInstr == nullptr) {
        std::cout << "[BB Error] One of instructions went nullptr "
                     "(InsertSingleInstrAfter)"
                  << std::endl;
        std::abort();
    }
    if (instToInsert->GetInstBB() == nullptr ||
        currentInstr->GetInstBB() == nullptr) {
        std::cout
            << "[BB Error] One of BB went nullptr (InsertSingleInstrAfter)"
            << std::endl;
        std::abort();
    }
    currentInstr->SetBB(this);
    auto *tmpNext = instToInsert->GetNextInst();
    instToInsert->SetNextInst(currentInstr);
    currentInstr->SetPrevInst(instToInsert);
    // currentInstr->SetNextInst(lastInstBB_);
    currentInstr->SetNextInst(tmpNext);
    tmpNext->SetPrevInst(currentInstr);

    if (!tmpNext) {
        lastInstBB_ = currentInstr;
    }
    size_ += 1;
}

template <bool PushBack> void BB::PushInstruction(SingleInstruction *instr) {
    assert((instr) && (instr->GetInstBB() == nullptr) &&
           (instr->GetPrevInst() == nullptr));
    instr->SetBB(this);

    if (instr->IsPhi()) {
        PushPhi(instr);
    } else if (firstInstBB_ == nullptr) {
        instr->SetPrevInst(lastPhiBB_);
        firstInstBB_ = instr;
        lastInstBB_ = instr;
        if (lastPhiBB_) {
            lastPhiBB_->SetNextInst(instr);
        }
    } else {
        if constexpr (PushBack) {
            instr->SetPrevInst(lastInstBB_);
            lastInstBB_->SetNextInst(instr);
            lastInstBB_ = instr;
        } else {
            instr->SetNextInst(firstInstBB_);
            firstInstBB_->SetPrevInst(instr);
            firstInstBB_ = instr;
        }
    }
    size_ += 1;
}

void BB::PushInstForward(SingleInstruction *instr) {
    PushInstruction<false>(instr);
}

void BB::PushInstBackward(SingleInstruction *instr) {
    PushInstruction<true>(instr);
}

void BB::PushPhi(SingleInstruction *instr) {
    if (instr == nullptr || !instr->IsPhi()) {
        std::cout << "[BB Error] PushPhi error." << std::endl;
    }
    if (firstPhiBB_ == nullptr) {
        firstPhiBB_ = reinterpret_cast<PhiInstr *>(instr);
        lastPhiBB_ = firstPhiBB_;
        lastPhiBB_->SetNextInst(firstInstBB_);
        if (firstInstBB_) {
            firstInstBB_->SetPrevInst(lastPhiBB_);
        }
    } else {
        instr->SetNextInst(firstPhiBB_);
        firstPhiBB_->SetPrevInst(instr);
        firstPhiBB_ = reinterpret_cast<PhiInstr *>(instr);
    }
}

bool BB::Domites(const BB *bblock) const {
    if (bblock == nullptr) {
        std::cout << "[BB Error] in Domites" << std::endl;
        std::abort();
    }
    auto *dom = bblock->GetDominator();
    while (dom != nullptr) {
        if (dom == this) {
            return true;
        }
        dom = dom->GetDominator();
    }
    return false;
}

void BB::PrintSSA() {
    std::cout << "Basic Block ID: " << GetId() << std::endl;

    // Print instructions in SSA format
    SingleInstruction *currentInst =
        GetFirstInstBB(); // Assuming this gets the first instruction
    while (currentInst) {
        currentInst
            ->PrintSSA(); // Call the PrintSSA method for each instruction
        currentInst =
            currentInst->GetNextInst(); // Move to the next instruction
    }

    // Print successors
    std::cout << "Successors: ";
    for (auto *succ : GetSuccessors()) {
        std::cout << "BB" << succ->GetId() << " "; // Print successor block IDs
    }
    std::cout << std::endl;
}

BB *JumpInstr::GetDestination() {
    auto *bblock = GetInstBB();
    assert(bblock);
    auto successors = bblock->GetSuccessors();
    assert(!successors.empty());
    return successors[0];
}

BB *CondJumpInstr::GetTrueDestination() {
    return getBranchDestinationImpl<0>();
}

BB *CondJumpInstr::GetFalseDestination() {
    return getBranchDestinationImpl<1>();
}

template <int CmpRes> BB *CondJumpInstr::getBranchDestinationImpl() {
    auto *bblock = GetInstBB();
    assert(bblock);
    auto successors = bblock->GetSuccessors();
    assert(successors.size() == 2);
    return successors[CmpRes];
}

} // namespace ir
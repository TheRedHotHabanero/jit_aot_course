#include "bb.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
namespace ir {
class SingleInstruction;
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
        predecessors_.erase(it);
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
    } else {
        firstInstBB_ = tmpNext;
    }

    if (tmpNext) {
        tmpNext->SetPrevInst(tmpPrev);
    } else {
        lastInstBB_ = tmpPrev;
    }

    CheckPhi(inst);
}

void BB::InsertSingleInstrBefore(SingleInstruction *instToInsert,
                                 SingleInstruction *currentInstr) {
    if (instToInsert == nullptr || currentInstr == nullptr) {
        std::cout << "[BB Error] One of instructions went nullptr "
                     "(InsertSingleInstrBefore)"
                  << std::endl;
        std::abort();
    }
    if (instToInsert->GetInstBB() == nullptr ||
        currentInstr->GetInstBB() == nullptr) {
        std::cout
            << "[BB Error] One of BB went nullptr (InsertSingleInstrBefore)"
            << std::endl;
        std::abort();
    }
    currentInstr->SetBB(this);
    auto *tmpPrev = instToInsert->GetPrevInst();
    instToInsert->SetPrevInst(currentInstr);
    currentInstr->SetPrevInst(tmpPrev);
    currentInstr->SetNextInst(instToInsert);

    if (!tmpPrev) {
        firstInstBB_ = currentInstr;
    }
    CheckPhi(currentInstr);
}

void BB::InsertSingleInstrAfter(SingleInstruction *instToInsert,
                                SingleInstruction *currentInstr) {
    std::cout << "here" << std::endl;
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
    currentInstr->SetNextInst(lastInstBB_);

    if (!tmpNext) {
        lastInstBB_ = currentInstr;
    }
    CheckPhi(currentInstr);
}

void BB::PushInstForward(SingleInstruction *instr) {
    if (!instr || instr->GetInstBB() != nullptr ||
        instr->GetPrevInst() != nullptr) {
        std::cout << "[BB Error] nullptr in 'PushInstForward'" << std::endl;
        std::abort();
    }

    // Set Parent bb for inst
    instr->SetBB(this);

    // If empty bb, set first and last instr
    if (firstInstBB_ == nullptr) {
        firstInstBB_ = instr;
        lastInstBB_ = instr;
    } else {
        instr->SetNextInst(firstInstBB_);
        firstInstBB_->SetPrevInst(instr);
        firstInstBB_ = instr;
    }

    CheckPhi(instr);
}

void BB::PushInstBackward(SingleInstruction *instr) {
    if (!instr || instr->GetInstBB() != nullptr ||
        instr->GetPrevInst() != nullptr) {
        std::cout << "[BB Error] nullptr in 'PushInstBackward'" << std::endl;
        std::abort();
    }

    // Set Parent bb for inst
    instr->SetBB(this);

    // If empty bb, set first and last instr
    if (firstInstBB_ == nullptr) {
        firstInstBB_ = instr;
        lastInstBB_ = instr;
    } else {
        instr->SetPrevInst(lastInstBB_);
        lastInstBB_->SetNextInst(instr);
        lastInstBB_ = instr;
    }

    CheckPhi(instr);
}

void BB::UpdFirstPhi() {
    SingleInstruction *instr = GetFirstInstBB();
    while (instr != nullptr) {
        if (instr->GetOpcode() == Opcode::PHI) {
            firstPhiBB_ = instr;
            break;
        }
        instr = instr->GetNextInst();
    }
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

} // namespace ir
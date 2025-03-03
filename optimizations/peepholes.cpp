#include <limits>
#include <cassert>
#include "peepholes.h"
#include "domTree/dfo_rpo.h"
#include "irGen/helperBuilderFunctions.h"

namespace ir {

void Peepholes::Run() {
    for (auto &bblock : RPO(graph_)) {
        for (auto *instr = bblock->GetFirstInstBB(); instr != nullptr;
             instr = instr->GetNextInst()) {
            switch (instr->GetOpcode()) {
                case Opcode::MUL:
                    VisitMul(instr);
                    break;
                case Opcode::SHR:
                    VisitShr(instr);
                    break;
                case Opcode::XOR:
                    VisitXor(instr);
                    break;
                default:
                    break;
            }
        }
    }
}

void Peepholes::VisitMul(SingleInstruction *inst) {
    assert(inst->GetOpcode() == Opcode::MUL);
    BinaryRegInstr *typed = static_cast<BinaryRegInstr *>(inst);

    if (constFolding_.ProcessMUL(typed)) {
        std::cout << "Folded MUL" << std::endl;
        return;
    }
    if (TryOptimizeMul(typed)) {
        return;
    }
}

void Peepholes::VisitShr(SingleInstruction *inst) {
    assert(inst->GetOpcode() == Opcode::SHR);
    BinaryRegInstr *typed = static_cast<BinaryRegInstr *>(inst);

    if (constFolding_.ProcessSHR(typed)) {
        std::cout << "Folded SHR" << std::endl;
        return;
    }

    if (TryOptimizeShr(typed)) {
        return;
    }
}

void Peepholes::VisitXor(SingleInstruction *inst) {
    assert(inst->GetOpcode() == Opcode::XOR);
    BinaryRegInstr *typed = static_cast<BinaryRegInstr *>(inst);
    
    if (TryOptimizeXor(typed)) {
        return;
    }
}

// MUL optimizations
bool Peepholes::TryOptimizeMul(BinaryRegInstr *inst) {
    auto input1 = inst->GetInput(0);
    auto input2 = inst->GetInput(1);
    
    // a * 1 => a
    if (input1->IsConst()) {
        auto *typed = static_cast<ConstInstr *>(input1.GetInstruction());
        if (typed->GetValue() == 1) {
            ReplaceWithoutNewInstr(inst, input2.GetInstruction());
            std::cout << "Applied MUL: '0 * v' peephole" << std::endl;
            return true;
        }
    }
    if (input2->IsConst()) {
        auto *typed = static_cast<ConstInstr *>(input2.GetInstruction());
        if (typed->GetValue() == 1) {
            ReplaceWithoutNewInstr(inst, input1.GetInstruction());
            std::cout << "Applied MUL: 'v * 0' peephole"<< std::endl;
            return true;
        }
    }
    
    // a * 0 => 0
    if (input1->IsConst()) {
        // case: v1 = 0 * v1 -> v1 = 0
        auto *typed = static_cast<ConstInstr *>(input1.GetInstruction());
        if (typed->GetValue() == 0) {
            ReplaceWithoutNewInstr(inst, input1.GetInstruction());
            std::cout << "Applied MUL: case: v1 = 0 * v1 -> v1 = 0 peephole" << std::endl;
        }
        return true;
    }
    if (input2->IsConst()) {
        // case: v1 = v0 * 0 -> v1 = 0
        auto *typed = static_cast<ConstInstr *>(input2.GetInstruction());
        if (typed->GetValue() == 0) {
            ReplaceWithoutNewInstr(inst, input2.GetInstruction());
            std::cout << "Applied MUL: case: v1 = v0 * 0 -> v1 = 0 peephole" << std::endl;
        }
        return true;
    }

    return false;
}

// SHR optimizations
bool Peepholes::TryOptimizeShr(BinaryRegInstr *inst) {
    auto input1 = inst->GetInput(0);
    auto input2 = inst->GetInput(1);
    if (input1->IsConst()) {
        auto *typed = static_cast<ConstInstr *>(input1.GetInstruction());
        if (typed->GetValue() == 0) {
            ReplaceWithoutNewInstr(inst, typed);
            std::cout << "Applied '0 >> v' peephole" << std::endl;
            return true;
        }
    }
    if (input2->IsConst()) {
        auto *typed = static_cast<ConstInstr *>(input2.GetInstruction());
        if (typed->GetValue() == 0) {
            ReplaceWithoutNewInstr(inst, input1.GetInstruction());
            std::cout << "Applied 'v >> 0' peephole" << std::endl;
            return true;
        }
    }

    // a >> n where n >= bit-width of a => 0
    if (input2->IsConst()) {
        auto *typed = static_cast<ConstInstr *>(input2.GetInstruction());
        if (typed->GetValue() >= 32) {
            auto *constZero = graph_->GetInstructionBuilder()->BuildConst(inst->GetType(), 0);
            ReplaceWithoutNewInstr(inst, constZero);
            std::cout << "Applied SHR: case: a >> n where n >= bit-width of a => 0 peephole" << std::endl;
            return true;
        }
    }

    return false;
}

// XOR optimizations
bool Peepholes::TryOptimizeXor(BinaryRegInstr *inst) {
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);

    if (input0->IsConst()) {
        auto *inputInstr = static_cast<ConstInstr *>(input0.GetInstruction());
        if (inputInstr->GetValue() == 0) {
            ReplaceWithoutNewInstr(inst, input1.GetInstruction());
            std::cout << "Applied XOR: '0 ^ v' => v" << std::endl;
            return true;
        }
    }
    if (input1->IsConst()) {
        auto *inputInstr = static_cast<ConstInstr *>(input1.GetInstruction());
        if (inputInstr->GetValue() == 0) {
            ReplaceWithoutNewInstr(inst, input0.GetInstruction());
            std::cout << "Applied XOR: 'v ^ 0' => v" << std::endl;
            return true;
        }
    }
    return false;
}

void Peepholes::ReplaceWithoutNewInstr(BinaryRegInstr *instr,
    SingleInstruction *replacedInstr) {
    assert(instr);
    instr->GetInstBB()->ReplaceInDataFlow(instr, replacedInstr);
    instr->GetInstBB()->SetInstructionAsDead(instr);

    // these instructions may be deleted later by DCE
    instr->GetInput(0)->RemoveUser(instr);
    instr->GetInput(1)->RemoveUser(instr);
}

}  // namespace ir

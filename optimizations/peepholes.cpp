#include <limits>
#include "peepholes.h"
#include "constFolding.h"
#include "/home/karina/2/jit_aot_course/irGen/helperBuilderFunctions.h"

namespace ir {

void Peepholes::Run() {
}

void Peepholes::AnalysisInst(SingleInstruction *inst) {
    switch (inst->GetOpcode()) {
        case Opcode::MULI:
            VisitMul(inst);
            break;
        case Opcode::SHRI:
            VisitShr(inst);
            break;
        case Opcode::XORI:
        case Opcode::XOR:
            VisitXor(inst);
            break;
        default:
            break;
    }
}

void Peepholes::VisitMul(SingleInstruction *inst) {
    if (ConstFoldingBinaryOp(graph_, inst)) {
        return;
    }
    TryOptimizeMul(inst);
}

void Peepholes::VisitShr(SingleInstruction *inst) {
    if (ir::ConstFoldingBinaryOp(graph_, inst)) {
        return;
    }
    TryOptimizeShr(inst);
}

void Peepholes::VisitXor(SingleInstruction *inst) {
    if (ConstFoldingBinaryOp(graph_, inst)) {
        return;
    }
    TryOptimizeXor(inst);
}

// MUL optimizations
bool Peepholes::TryOptimizeMul(SingleInstruction *inst) {
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    
    // a * 0 => 0
    if (input1->GetInstruction()->IsConst() && input1->GetInstruction()->CastToConstant()->GetValue() == 0) {
        std::cout << "================= mul opt 1" << std::endl;
        ReplaceInstructionWithConstant(inst, 0);
        return true;
    }
    
    // a * 1 => a
    if (input1->GetInstruction()->IsConst() && input1->GetInstruction()->CastToConstant()->GetValue() == 1) {
        std::cout << "================= mul opt 2" << std::endl;
        ReplaceInstructionWithInput(inst, input0);
        return true;
    }

    return false;
}

// SHR optimizations
bool Peepholes::TryOptimizeShr(SingleInstruction *inst) {
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);

    // a >> 0 => a
    if (input1->GetInstruction()->IsConst() && input1->GetInstruction()->CastToConstant()->GetValue() == 0) {
        std::cout << "================= shr opt 1" << std::endl;
        ReplaceInstructionWithInput(inst, input0);
        return true;
    }

    // a >> n where n >= bit-width of a => 0
    if (input1->GetInstruction()->IsConst() &&
        input1->GetInstruction()->CastToConstant()->GetValue() >= 32) {
        std::cout << "================= shr opt 2" << std::endl;
        ReplaceInstructionWithConstant(inst, 0);
        return true;
    }

    return false;
}

// XOR optimizations
bool Peepholes::TryOptimizeXor(SingleInstruction *inst) {
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);

    // a ^ 0 => a
    if (input1->GetInstruction()->IsConst() && input1->GetInstruction()->CastToConstant()->GetValue() == 0) {
        std::cout << "================= xor opt 1" << std::endl;
        ReplaceInstructionWithInput(inst, input0);
        return true;
    }

    // a ^ a => 0
    if (input0->GetInstruction() == input1->GetInstruction()) {
        std::cout << "================= xor opt 2" << std::endl;
        ReplaceInstructionWithConstant(inst, 0);
        return true;
    }

    return false;
}

void Peepholes::ReplaceInstructionWithInput(SingleInstruction *inst, Input *input) {
    inst->GetInstBB()->ReplaceInstruction(inst, input->GetInstruction());
}

void Peepholes::ReplaceInstructionWithConstant(SingleInstruction *inst, int32_t constantValue) {
    auto *newConstInst = new ConstInstr(
        Opcode::CONST, InstType::i32,
        static_cast<uint64_t>(constantValue));
    inst->GetInstBB()->ReplaceInstruction(inst, newConstInst);
}

}  // namespace ir

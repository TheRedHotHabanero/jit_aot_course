#include "constFolding.h"
#include "/home/karina/2/jit_aot_course/irGen/graph.h"
#include "/home/karina/2/jit_aot_course/irGen/singleInstruction.h"
#include "/home/karina/2/jit_aot_course/irGen/helperBuilderFunctions.h"

namespace ir {

    bool ConstFoldingBinaryOp([[maybe_unused]]Graph *graph, SingleInstruction *inst) {
        auto input0 = inst->GetInput(0);
        auto input1 = inst->GetInput(1);
        if (!input0->GetInstruction()->IsConst() || !input1->GetInstruction()->IsConst()) {
            return false;
        }
        
        SingleInstruction *new_const = nullptr;
        auto imm0 = input0->GetInstruction()->CastToConstant()->GetValue();
        auto imm1 = input1->GetInstruction()->CastToConstant()->GetValue();
        InstructionBuilder instructionBuilder;
        auto instType = InstType::i32;
    
        switch (inst->GetOpcode()) {
            case Opcode::MUL:
                new_const = instructionBuilder.BuildConst(instType, imm0 * imm1);
                break;
            case Opcode::SHR:
                new_const = instructionBuilder.BuildConst(instType, imm0 >> imm1);
                break;
            case Opcode::XOR:
                new_const = instructionBuilder.BuildConst(instType, imm0 ^ imm1);
                break;
            default:
                std::cerr << "Unsupported instruction" << std::endl;
        }

        new_const->ReplaceWith(inst);
    
        return true;
    }
    
    } // namespace ir
    
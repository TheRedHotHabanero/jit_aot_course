#include "constFolding.h"
#include "irGen/helperBuilderFunctions.h"


namespace ir {
bool ConstantFolding::ProcessMUL(BinaryRegInstr *instr) {
    assert((instr) && (instr->GetOpcode() == Opcode::MUL));
    auto input1 = instr->GetInput(0);
    auto input2 = instr->GetInput(1);
    if (input1->IsConst() && input2->IsConst()) {
        auto value = AsConst(input1.GetInstruction())->GetValue() * AsConst(input2.GetInstruction())->GetValue();
        auto *newInstr = GetInstructionBuilder(instr)->BuildConst(instr->GetType(), value);

        input1->RemoveUser(instr);
        input2->RemoveUser(instr);
        instr->GetInstBB()->ReplaceInstruction(instr, newInstr);

        return true;
    }
    return false;
}

bool ConstantFolding::ProcessSHR(BinaryRegInstr *instr) {
    assert((instr) && (instr->GetOpcode() == Opcode::SHR));
    auto input1 = instr->GetInput(0);
    auto input2 = instr->GetInput(1);
    if (input1->IsConst() && input2->IsConst()) {
        auto value = ToSigned(AsConst(input1.GetInstruction())->GetValue(), instr->GetType()) >> AsConst(input2.GetInstruction())->GetValue();
        auto *newInstr = GetInstructionBuilder(instr)->BuildConst(instr->GetType(), value);

        input1->RemoveUser(instr);
        input2->RemoveUser(instr);
        instr->GetInstBB()->ReplaceInstruction(instr, newInstr);

        return true;
    }
    return false;
}

bool ConstantFolding::ProcessXOR(BinaryRegInstr *instr) {
    assert((instr) && (instr->GetOpcode() == Opcode::XOR));
    auto input1 = instr->GetInput(0);
    auto input2 = instr->GetInput(1);
    if (input1->IsConst() && input2->IsConst()) {
        auto value = AsConst(input1.GetInstruction())->GetValue() ^ AsConst(input2.GetInstruction())->GetValue();
        auto *newInstr = GetInstructionBuilder(instr)->BuildConst(instr->GetType(), value);

        input1->RemoveUser(instr);
        input2->RemoveUser(instr);
        instr->GetInstBB()->ReplaceInstruction(instr, newInstr);

        return true;
    }
    return false;
}

ConstInstr *ConstantFolding::AsConst(SingleInstruction *instr) {
    assert((instr) && instr->IsConst());
    return static_cast<ConstInstr *>(instr);
}

InstructionBuilder *ConstantFolding::GetInstructionBuilder(SingleInstruction *instr) {
    return instr->GetInstBB()->GetGraph()->GetInstructionBuilder();
}
}   // namespace ir
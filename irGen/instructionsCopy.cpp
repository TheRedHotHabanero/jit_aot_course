#include "helperBuilderFunctions.h"

namespace ir {

BinaryImmInstr *BinaryImmInstr::Copy(BB *targetBBlock) {
    auto *allocator = targetBBlock->GetGraph()->GetAllocator();
    auto *instr = allocator->template New<BinaryImmInstr>(
        GetOpcode(), GetType(), GetInput(0), GetValue(), allocator);
    targetBBlock->GetGraph()->GetInstructionBuilder()->AttachInstruction(instr);
    instr->SetProperty(GetProperties());
    return instr;
}

BinaryRegInstr *BinaryRegInstr::Copy(BB *targetBBlock) {
    auto *allocator = targetBBlock->GetGraph()->GetAllocator();
    auto *instr = allocator->template New<BinaryRegInstr>(
        GetOpcode(), GetType(), GetInput(0), GetInput(1), allocator);
    targetBBlock->GetGraph()->GetInstructionBuilder()->AttachInstruction(instr);
    instr->SetProperty(GetProperties());
    return instr;
}

UnaryRegInstr *UnaryRegInstr::Copy(BB *targetBBlock) {
    auto *allocator = targetBBlock->GetGraph()->GetAllocator();
    auto *instr = allocator->template New<UnaryRegInstr>(
        GetOpcode(), GetType(), GetInput(0), allocator);
    targetBBlock->GetGraph()->GetInstructionBuilder()->AttachInstruction(instr);
    instr->SetProperty(GetProperties());
    return instr;
}

LoadImmInstr *LoadImmInstr::Copy(BB *targetBBlock) {
    auto *allocator = targetBBlock->GetGraph()->GetAllocator();
    auto *instr = allocator->template New<LoadImmInstr>(
        GetOpcode(), GetType(), nullptr, GetValue(), allocator);
    targetBBlock->GetGraph()->GetInstructionBuilder()->AttachInstruction(instr);
    instr->SetProperty(GetProperties());
    return instr;
}

StoreImmInstr *StoreImmInstr::Copy(BB *targetBBlock) {
    auto *allocator = targetBBlock->GetGraph()->GetAllocator();
    auto *instr = allocator->template New<StoreImmInstr>(
        GetOpcode(), nullptr, nullptr, GetValue(), allocator);
    targetBBlock->GetGraph()->GetInstructionBuilder()->AttachInstruction(instr);
    instr->SetProperty(GetProperties());
    return instr;
}

ConstInstr *ConstInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildConst(GetType(), GetValue());
}

CastInstr *CastInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildCast(GetType(), GetTargetType(), GetInput(0));
}

CompInstr *CompInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildCmp(GetType(), GetCondCode(), GetInput(0),
                             GetInput(1));
}

CondJumpInstr *CondJumpInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildJcmp();
}

JumpInstr *JumpInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildJmp();
}

RetInstr *RetInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildRet(GetType(), GetInput(0));
}

RetVoidInstr *RetVoidInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildRetVoid();
}

CallInstr *CallInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildCall(GetType(), GetCallTarget(), GetInputs());
}

PhiInstr *PhiInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildPhi(GetType(), GetInputs(), GetSourceBBs());
}

InputArgInstr *InputArgInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildArg(GetType());
}

LengthInstr *LengthInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildLen(nullptr);
}

NewArrayInstr *NewArrayInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildNewArray(nullptr, GetTypeId());
}

NewArrayImmInstr *NewArrayImmInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildNewArrayImm(GetValue(), GetTypeId());
}

BoundsCheckInstr *BoundsCheckInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildBoundsCheck(nullptr, nullptr);
}

NewObjectInstr *NewObjectInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildNewObject(GetTypeId());
}

LoadArrayInstr *LoadArrayInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildLoadArray(GetType(), nullptr, nullptr);
}

StoreArrayInstr *StoreArrayInstr::Copy(BB *targetBBlock) {
    auto *builder = targetBBlock->GetGraph()->GetInstructionBuilder();
    return builder->BuildStoreArray(nullptr, nullptr, nullptr);
}

}; // namespace ir
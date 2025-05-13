#include "singleInstruction.h"
#include "bb.h"
#include <cstdlib>
#include <iostream>
namespace ir {

InstType getInstType(const std::type_info &type) {
    if (type == typeid(int8_t)) {
        return InstType::i8;
    }
    if (type == typeid(int16_t)) {
        return InstType::i16;
    }
    if (type == typeid(int32_t)) {
        return InstType::i32;
    }
    if (type == typeid(int64_t)) {
        return InstType::i64;
    }
    if (type == typeid(uint8_t)) {
        return InstType::u8;
    }
    if (type == typeid(uint16_t)) {
        return InstType::u16;
    }
    if (type == typeid(uint32_t)) {
        return InstType::u32;
    }
    if (type == typeid(uint64_t)) {
        return InstType::u64;
    }
    if (type == typeid(void)) {
        return InstType::VOID;
    }

    std::cout << "[Operand Error] Operand type is out of possible values."
              << std::endl;
    return InstType::INVALID;
}

const char *SingleInstruction::GetOpcodeName(Opcode opcode) const {
    if (static_cast<size_t>(opcode) < nameOpcode.size()) {
        return nameOpcode[static_cast<size_t>(opcode)];
    } else {
        return "INVALID";
    }
}

void SingleInstruction::RemoveFromBlock() {
    if (instBB_ == nullptr) {
        std::cout << "[SingleInstruction Error] Nullptr in BB containing "
                     "current inst "
                     "(RemoveFromBlock)."
                  << std::endl;
        std::abort();
    }
    instBB_->SetInstructionAsDead(this);
}

bool SingleInstruction::Dominates(SingleInstruction *other) {
    assert(other);
    if (other == this) {
        // instruction always dominates itself
        return true;
    }
    auto *otherBlock = other->GetInstBB();
    if (otherBlock == GetInstBB()) {
        return IsEarlierInBasicBlock(other);
    }
    return GetInstBB()->Domites(otherBlock);
}

bool SingleInstruction::IsEarlierInBasicBlock(SingleInstruction *other) {
    assert((other) && (GetInstBB()) && other->GetInstBB() == GetInstBB());
    for (auto *instr : *GetInstBB()) {
        if (instr == this) {
            return true;
        }
        if (instr == other) {
            return false;
        }
    }
    std::cerr << "[SingleInstruction Error] Must have encountered either this "
                 "or other instruction"
              << std::endl;
    return false;
}

void SingleInstruction::InsertInstBefore(SingleInstruction *inst) {
    if (instBB_ == nullptr) {
        std::cout << "[SingleInstruction Error] Nullptr BB while trying "
                     "inserting inst before."
                  << std::endl;
        std::abort();
    }
    instBB_->InsertSingleInstrBefore(inst, this);
}

void SingleInstruction::InsertInstAfter(SingleInstruction *inst) {
    if (instBB_ == nullptr) {
        std::cout << "[SingleInstruction Error] Nullptr BB while trying "
                     "inserting inst after."
                  << std::endl;
        std::abort();
    }
    instBB_->InsertSingleInstrAfter(inst, this);
}

class ConstInstr;
ConstInstr *SingleInstruction::CastToConstant() {
    if (GetOpcode() != Opcode::CONST) {
        std::cerr << "Non-const inst for CastToConstant!!" << std::endl;
    }
    return static_cast<ConstInstr *>(this);
}
class InputsInstr;
void SingleInstruction::ReplaceInputInUsers(SingleInstruction *newInput) {
    newInput->AddUsers(GetUsers());
    for (auto &it : GetUsers()) {
        // assert(it->HasInputs());
        auto *typed = static_cast<InputsInstr *>(it);
        typed->ReplaceInput(this, newInput);
    }
}

void SingleInstruction::PrintSSA() {
    std::cout << "  ";
    std::cout << GetOpcodeName(GetOpcode()) << " ";
    std::cout << "%" << instID_;

    // Print operands
    std::cout << std::endl;
}

} // namespace ir
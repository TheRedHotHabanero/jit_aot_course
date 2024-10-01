#ifndef JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_
#define JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_

#include "graph.h"
#include <vector>

namespace ir {

class InstructionBuilder {

public:
    InstructionBuilder() = default;
    InstructionBuilder(const InstructionBuilder &) = delete;
    InstructionBuilder &operator=(const InstructionBuilder &) = delete;
    InstructionBuilder(InstructionBuilder &&) = delete;
    InstructionBuilder &operator=(InstructionBuilder &&) = delete;
    virtual ~InstructionBuilder() noexcept { Clear(); }

    void PushBackInst(BB *bb, SingleInstruction *instr) {
        bb->PushInstBackward(instr);
    }

    void PushForwardInst(BB *bb, SingleInstruction *instr) {
        bb->PushInstForward(instr);
    }

    SingleInstruction *GetLastInst() {
        return instructions_.back();
    }

    void Clear() noexcept {
        for (auto *instr : instructions_) {
            delete instr;
        }
        instructions_.clear();
    }

    // Refactored methods for instruction creation
    SingleInstruction* BuildSingleReg(Opcode opcode, InstType type, VReg vdest, VReg vreg) {
        auto* inst = new SingleInstruction(opcode, type);
        inst->SetVirtualReg(vdest);
        inst->SetVirtualReg1(vreg);
        instructions_.push_back(inst);
        return inst;
    }

    SingleInstruction* BuildTwoReg(Opcode opcode, InstType type, VReg vdest, VReg vreg1, VReg vreg2) {
        auto* inst = new SingleInstruction(opcode, type);
        inst->SetVirtualReg(vdest);
        inst->SetVirtualReg1(vreg1);
        inst->SetVirtualReg2(vreg2);
        instructions_.push_back(inst);
        return inst;
    }

    SingleInstruction* BuildTwoImm(Opcode opcode, InstType type, VReg vdest, VReg vreg, VReg imm) {
        auto* inst = new SingleInstruction(opcode, type);
        inst->SetVirtualReg(vdest);
        inst->SetVirtualReg1(vreg);
        inst->SetVirtualReg2(imm); // Assuming inst can hold an immediate value
        instructions_.push_back(inst);
        return inst;
    }

    SingleInstruction* BuildMovi(InstType type, VReg vdest, uint64_t imm) {
        return BuildTwoImm(Opcode::MOVI, type, vdest, VReg(), VReg(imm)); // Assuming VReg() represents a null or default state
    }

    SingleInstruction* BuildAddi(InstType type, VReg vdest, VReg vreg, uint64_t imm) {
        auto* inst = BuildTwoImm(Opcode::ADDI, type, vdest, vreg, VReg(imm));
        instructions_.push_back(inst);
        return inst;
    }

    SingleInstruction* BuildMul(InstType type, VReg vdest, VReg vreg, uint64_t imm) {
        auto* inst = BuildTwoImm(Opcode::MUL, type, vdest, vreg, VReg(imm));
        instructions_.push_back(inst);
        return inst;
    }

    SingleInstruction* BuildCmp(InstType type, Conditions condition, VReg v1, VReg v2) {
        auto* inst = BuildTwoReg(Opcode::CMP, type, VReg(), v1, v2); // Использовать значение по умолчанию для dest
        inst->SetVirtualReg1(v1); // Предполагается, что у вас есть методы для установки регистров
        inst->SetVirtualReg2(v2);
        instructions_.push_back(inst);
        inst->SetCondCode(condition);
        return inst;
    }

    SingleInstruction* BuildCast(InstType fromType, InstType targetType, VReg vdest, VReg vreg) {
        auto* inst = BuildSingleReg(Opcode::CAST, fromType, vdest, vreg);
        inst->SetRegType(targetType); // Assuming a method to set target type
        instructions_.push_back(inst);
        return inst;
    }

    SingleInstruction* BuildJmp(uint64_t imm) {
        auto* inst = new SingleInstruction(Opcode::JMP, InstType::i64);
        inst->SetVirtualReg2(VReg(imm)); // Предполагается, что метод SetVirtualReg2 может принимать немедленное значение
        instructions_.push_back(inst);
        return inst;
    }

    SingleInstruction* BuildJa(uint64_t imm) {
        auto* inst = new SingleInstruction(Opcode::JA, InstType::i64);
        inst->SetVirtualReg2(VReg(imm)); // Предполагается, что метод SetVirtualReg2 может принимать немедленное значение
        instructions_.push_back(inst);
        return inst;
    }

    SingleInstruction* BuildRet(InstType type, VReg vreg) {
        return BuildSingleReg(Opcode::RET, type, vreg, VReg());
    }

    SingleInstruction* BuildPhi(InstType type, VReg vdest, VReg vreg1, VReg vreg2) {
        return BuildTwoReg(Opcode::PHI, type, vdest, vreg1, vreg2);
    }

private:
    std::vector<SingleInstruction *> instructions_;
};

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_

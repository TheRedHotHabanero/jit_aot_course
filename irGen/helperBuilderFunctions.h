#ifndef JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_
#define JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_

#include "graph.h"
#include <vector>

namespace ir {

// Who contains results-----------------------------------------------------
class DestIsVirtualReg {
  public:
    DestIsVirtualReg(VReg vreg) : destReg_(vreg) {}
    auto GetDestVReg() { return destReg_; }
    void SetDestVReg(VReg vreg) { destReg_ = vreg; }

  private:
    VReg destReg_;
};

template <typename T> class DestIsImm {
  public:
    explicit DestIsImm(T value) : destImm_(value) {}
    auto GetImm() { return destImm_; }
    void SetImm(T value) { destImm_ = value; }

  private:
    T destImm_;
};

class DestCondition {
  public:
    DestCondition(Conditions condition) : condition_(condition) {}
    auto GetCondCode() const { return condition_; }
    void SetCondCode(Conditions condition) { condition_ = condition; }

  private:
    Conditions condition_;
};
// ----------------------------------------------------------------------------------

// Specific Instruction containers according to Opcode
// list--------------------------------

class SingleRegInst : public SingleInstruction, public DestIsVirtualReg {
  public:
    SingleRegInst(Opcode opcode, InstType type, VReg vdest, VReg vreg)
        : SingleInstruction(opcode, type), DestIsVirtualReg(vdest),
          vreg_(vreg) {}

    auto GetVReg() { return vreg_; }
    void SetVReg(VReg newVReg) { vreg_ = newVReg; }

  private:
    VReg vreg_;
};

class TwoRegInst : public SingleInstruction, public DestIsVirtualReg {
  public:
    TwoRegInst(Opcode opcode, InstType type, VReg vdest, VReg vreg1, VReg vreg2)
        : SingleInstruction(opcode, type), DestIsVirtualReg(vdest),
          vreg1_(vreg1), vreg2_(vreg2) {}

    auto GetVReg1() { return vreg1_; }
    auto GetVReg2() { return vreg2_; }
    void SetVReg1(VReg newVReg) { vreg1_ = newVReg; }
    void SetVReg2(VReg newVReg) { vreg2_ = newVReg; }

  private:
    VReg vreg1_;
    VReg vreg2_;
};

class TwoImmInst : public SingleInstruction,
                   public DestIsVirtualReg,
                   public DestIsImm<uint64_t> {
  public:
    TwoImmInst(Opcode opcode, InstType type, VReg vdest, VReg vreg,
               uint64_t imm)
        : SingleInstruction(opcode, type),
          DestIsVirtualReg(vdest), DestIsImm<uint64_t>(imm), vreg_(vreg) {}

    auto GetVReg() { return vreg_; }
    void SetVReg(VReg newVReg) { vreg_ = newVReg; }

  private:
    VReg vreg_;
};

class MoveImmInst : public SingleInstruction,
                    public DestIsVirtualReg,
                    public DestIsImm<uint64_t> {
  public:
    MoveImmInst(Opcode opcode, InstType type, VReg vdest, uint64_t imm)
        : SingleInstruction(opcode, type),
          DestIsVirtualReg(vdest), DestIsImm<uint64_t>(imm) {}
};

class CompInstr : public SingleInstruction, public DestCondition {
  public:
    CompInstr(Opcode opcode, InstType type, Conditions confition, VReg v1,
              VReg v2)
        : SingleInstruction(opcode, type), DestCondition(confition), vreg1_(v1),
          vreg2_(v2) {}

    auto GetVReg1() { return vreg1_; }
    auto GetVReg2() { return vreg2_; }
    void SetVReg1(VReg newVReg) { vreg1_ = newVReg; }
    void SetVReg2(VReg newVReg) { vreg2_ = newVReg; }

  private:
    VReg vreg1_;
    VReg vreg2_;
};

class CastInstr : public SingleRegInst {
  public:
    explicit CastInstr(InstType fromType, InstType targetType, VReg vdest,
                       VReg vreg)
        : SingleRegInst(Opcode::CAST, fromType, vdest, vreg),
          targetType_(targetType) {}

    auto GetTargetType() { return targetType_; }
    void SetTargetType(InstType newType) { targetType_ = newType; }

  private:
    InstType targetType_;
};

class JumpInstr : public SingleInstruction, public DestIsImm<uint64_t> {
  public:
    JumpInstr(Opcode opcode, uint64_t imm)
        : SingleInstruction(opcode, InstType::i64), DestIsImm<uint64_t>(imm) {}
};

class RetInstr : public SingleInstruction {
  public:
    RetInstr(InstType type, VReg vreg)
        : SingleInstruction(Opcode::RET, type), vreg_(vreg) {}

    auto GetVReg() { return vreg_; }
    void SetVReg(VReg newVReg) { vreg_ = newVReg; }

  private:
    VReg vreg_;
};

class PhiInstr : public TwoRegInst {
  public:
    PhiInstr(InstType type, VReg vdest, VReg vreg1, VReg vreg2)
        : TwoRegInst(Opcode::PHI, type, vdest, vreg1, vreg2) {}
};

// ------------------------------------------------------------------------------------------

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
        return instructions_[instructions_.size() - 1];
    }

    void Clear() noexcept {
        for (auto *instr : instructions_) {
            delete instr;
        }
        instructions_.clear();
    }

  private:
    std::vector<SingleInstruction *> instructions_; //добавить номера инструкций, но не int сделать

  public:
    TwoRegInst *BuildMul(InstType type, VReg vdest, VReg v1, VReg v2) {
        auto *inst = new TwoRegInst(Opcode::MUL, type, vdest, v1, v2);
        instructions_.push_back(inst);
        return inst;
    }

    template <typename T>
    TwoImmInst *BuildAddi(InstType type, VReg vdest, VReg vreg, T imm) {
        auto *inst = new TwoImmInst(Opcode::ADDI, type, vdest, vreg, imm);
        instructions_.push_back(inst);
        return inst;
    }

    template <typename T>
    MoveImmInst *BuildMovi(InstType type, VReg vdest, T imm) {
        auto *inst = new MoveImmInst(Opcode::MOVI, type, vdest, imm);
        instructions_.push_back(inst);
        return inst;
    }

    CastInstr *BuildCast(InstType fromType, InstType targetType, VReg vdest,
                         VReg vreg) {
        auto *inst = new CastInstr(fromType, targetType, vdest, vreg);
        instructions_.push_back(inst);
        return inst;
    }

    CompInstr *BuildCmp(InstType type, Conditions conditions, VReg v1,
                        VReg v2) {
        auto *inst = new CompInstr(Opcode::CMP, type, conditions, v1, v2);
        instructions_.push_back(inst);
        return inst;
    }

    JumpInstr *BuildJa(int64_t imm) {
        auto *inst = new JumpInstr(Opcode::JA, imm);
        instructions_.push_back(inst);
        return inst;
    }

    JumpInstr *BuildJmp(int64_t imm) {
        auto *inst = new JumpInstr(Opcode::JMP, imm);
        instructions_.push_back(inst);
        return inst;
    }

    RetInstr *BuildRet(InstType type, VReg vreg) {
        auto *inst = new RetInstr(type, vreg);
        instructions_.push_back(inst);
        return inst;
    }

    PhiInstr *BuildPhi(InstType type, VReg vdest, VReg vreg1, VReg vreg2) {
        auto *inst = new PhiInstr(type, vdest, vreg1, vreg2);
        instructions_.push_back(inst);
        return inst;
    }
};
} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_

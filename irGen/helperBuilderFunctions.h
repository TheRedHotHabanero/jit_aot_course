#ifndef JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_
#define JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_

#include "graph.h"
#include "instructions.h"
#include <cstdint>
#include <type_traits>
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
        return instructions_[instructions_.size() - 1];
    }

    void Clear() noexcept {
        for (auto *instr : instructions_) {
            delete instr;
        }
        instructions_.clear();
    }

  private:
    std::vector<SingleInstruction *> instructions_;

  public:
    TwoRegInst *BuildMul(InstType type, Input input1, Input input2) {
        auto *inst = new TwoRegInst(Opcode::MUL, type, input1, input2);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    template <typename T,
              typename = std::enable_if_t<
                  std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                  std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>>>
    TwoRegInst *BuildAddi(InstType type, Input input, T immediate) {
        auto *constInstr = new ConstInstr(
            Opcode::CONST, type,
            static_cast<uint64_t>(immediate)); // Используем ConstInstr
        Input immInput = Input(constInstr); // Оборачиваем в Input
        auto *inst = new TwoRegInst(Opcode::ADDI, type, input, immInput);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    template <typename T,
              typename = std::enable_if_t<
                  std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                  std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>>>
    ConstInstr *BuildConst(InstType type, T imm) {
        auto *inst = new ConstInstr(Opcode::CONST, type, imm);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    CastInstr *BuildCast(InstType fromType, InstType targetType, Input input) {
        auto *inst = new CastInstr(fromType, targetType, input);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    CompInstr *BuildCmp(InstType type, Conditions conditions, Input input1,
                        Input input2) {
        auto *inst =
            new CompInstr(Opcode::CMP, type, conditions, input1, input2);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    JumpInstr *BuildJa(int64_t imm = -1) {
        auto *inst = new JumpInstr(Opcode::JA, imm);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    JumpInstr *BuildJmp(int64_t imm) {
        auto *inst = new JumpInstr(Opcode::JMP, imm);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    RetInstr *BuildRet(InstType type, Input input) {
        auto *inst = new RetInstr(type, input);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    PhiInstr *BuildPhi(InstType type) {
        auto *inst = new PhiInstr(type);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    template <typename... T> PhiInstr *BuildPhi(InstType type, T... inputs) {
        auto *inst = new PhiInstr(type, inputs...);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    InputArgInstr *BuildArg(InstType type) {
        auto *inst = new InputArgInstr(type);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }
};
} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_

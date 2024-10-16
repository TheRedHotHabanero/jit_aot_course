/*
    Instruction structure is stolen from lecture

                                                   Basic Block
    ____________________________________________________________________________________________________________________
    | Instruction 1                Instruction 2                                                        Instruction n   |
    | - prev                       - prev           (intrusive linked list)                             - prev          |
    | - next                       - next           (intrusive linked list)          |       |          - next          |
    | - opcode           <-->      - opcode         (common part)              <---> | <---> | <--->    - opcode        |
    | - type                       - type           (common part)                    |       |          - type          |
    | - basic block                - basic block    (common part)                                       - basic block   |
    | - inputs                     - inputs         (derived part)                                      - inputs        |
    ____________________________________________________________________________________________________________________|



    Id              - uniq number of inst. If phi -> id contains "p"
    Type            - type of result of inst (s32, u64, ...)
    Opcode          - name of instruction
    Basic flags     - basic inst properties (throwable, no_dce, ...)
    Special flags   - special properties for inst (conditional code, inlined, ...)
    Inputs          - array of instructions, which need to execute the instruction
    Users           - list of instructions, which have instuction as input
*/

#ifndef JIT_AOT_COURSE_IR_GEN_INSTRUCTIONS_H_
#define JIT_AOT_COURSE_IR_GEN_INSTRUCTIONS_H_

#include "input.h"
#include "singleInstruction.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
namespace ir {
class BB;

enum class Conditions { EQ, NONEQ, LSTHAN, GRTHAN };

class InputsInstr : public SingleInstruction {
  public:
    InputsInstr(Opcode opcode, InstType type)
        : SingleInstruction(opcode, type) {}
    ~InputsInstr() = default;

    virtual Input &GetInput(size_t idx) = 0;
    virtual void SetInput(Input newInput, size_t idx) = 0;
};

template <int InputsNum> class ConstInputsInst : public InputsInstr {
  public:
    ConstInputsInst(Opcode opcode, InstType type) : InputsInstr(opcode, type) {}

    template <typename... T>
    ConstInputsInst(Opcode opcode, InstType type, T... inputs)
        : InputsInstr(opcode, type), inputs_{inputs...} {}

    Input &GetInput(size_t idx) override { return inputs_.at(idx); }
    void SetInput(Input newInput, size_t idx) override {
        inputs_.at(idx) = newInput;
    }

    std::array<Input, InputsNum> &GetInputs() { return inputs_; }

  private:
    std::array<Input, InputsNum> inputs_;
};

template <> class ConstInputsInst<1> : public InputsInstr {
  public:
    ConstInputsInst(Opcode opcode, InstType type) : InputsInstr(opcode, type) {}
    ConstInputsInst(Opcode opcode, InstType type, Input input)
        : InputsInstr(opcode, type), input_(input) {}

    Input &GetInput() { return input_; }
    const Input &GetInput() const { return input_; }
    Input &GetInput(size_t idx) override {
        if (idx <= 0) {
            std::cout << "[Inst Error] in GetInput" << std::endl;
            std::abort();
        }
        return input_;
    }
    void SetInput(Input newInput, size_t idx) override {
        if (idx <= 0) {
            std::cout << "[Inst Error] in SetInput" << std::endl;
            std::abort();
        }
        input_ = newInput;
    }

  private:
    Input input_;
};

class VarInputsInstr : public InputsInstr {
  public:
    VarInputsInstr(Opcode opcode, InstType type) : InputsInstr(opcode, type) {}

    ~VarInputsInstr() = default;

    Input &GetInput(size_t idx) override { return inputs_.at(idx); }
    void SetInput(Input newInput, size_t idx) override {
        inputs_.at(idx) = newInput;
    }

    std::vector<Input> &GetInputs() { return inputs_; }
    void AddInput(Input newInput) { inputs_.push_back(newInput); }

  private:
    std::vector<Input> inputs_;
};

// Who contains results-----------------------------------------------------
template <typename T> class DestIsImm {
  public:
    explicit DestIsImm(T value) : destImm_(value) {}
    auto GetValue() { return destImm_; }
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
class SingleRegInst : public ConstInputsInst<1> {
  public:
    SingleRegInst(Opcode opcode, InstType type, Input input)
        : ConstInputsInst(opcode, type, input) {}
};

class TwoRegInst : public ConstInputsInst<2> {
  public:
    TwoRegInst(Opcode opcode, InstType type, Input in1, Input in2)
        : ConstInputsInst(opcode, type, in1, in2) {}
};

class ConstInstr : public SingleInstruction, public DestIsImm<uint64_t> {
  public:
    ConstInstr(Opcode opcode, InstType type)
        : SingleInstruction(opcode, type), DestIsImm<uint64_t>(0) {}
    ConstInstr(Opcode opcode, InstType type, uint64_t value)
        : SingleInstruction(opcode, type), DestIsImm<uint64_t>(value) {}
};

class BinaryImmInstr : public ConstInputsInst<1>, public DestIsImm<uint64_t> {
  public:
    BinaryImmInstr(Opcode opcode, InstType type, Input input, uint64_t imm)
        : ConstInputsInst<1>(opcode, type, input), DestIsImm<uint64_t>(imm) {}
};

class CompInstr : public ConstInputsInst<2>, public DestCondition {
  public:
    CompInstr(Opcode opcode, InstType type, Conditions ccode, Input in1,
              Input in2)
        : ConstInputsInst(opcode, type, in1, in2), DestCondition(ccode) {}
};

class CastInstr : public ConstInputsInst<1> {
  public:
    CastInstr(InstType fromType, InstType toType, Input input)
        : ConstInputsInst(Opcode::CAST, fromType, input), toType_(toType) {}

    auto GetTargetType() const { return toType_; }
    void SetTargetType(InstType newType) { toType_ = newType; }

  private:
    InstType toType_;
};

class JumpInstr : public SingleInstruction, public DestIsImm<uint64_t> {
  public:
    JumpInstr(Opcode opcode, uint64_t imm)
        : SingleInstruction(opcode, InstType::i64), DestIsImm<uint64_t>(imm) {}
};

class RetInstr : public ConstInputsInst<1> {
  public:
    RetInstr(InstType type, Input input)
        : ConstInputsInst<1>(Opcode::RET, type, input) {}
};

class PhiInstr : public VarInputsInstr {
  public:
    explicit PhiInstr(InstType type) : VarInputsInstr(Opcode::PHI, type) {}

    template <typename... T>
    PhiInstr(InstType type, T... input)
        : VarInputsInstr(Opcode::PHI, type, input...) {}
};

class InputArgInstr : public SingleInstruction {
  public:
    explicit InputArgInstr(InstType type)
        : SingleInstruction(Opcode::ARG, type) {}
};

// ------------------------------------------------------------------------------------------

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_INSTRUCTIONS_H_

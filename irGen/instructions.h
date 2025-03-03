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
#include "domTree/arena.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
namespace ir {
class BB;
using memory::ArenaAllocator;

enum class Conditions { EQ, NONEQ, LSTHAN, GRTHAN };

class InputsInstr : public SingleInstruction {
  public:
    InputsInstr(Opcode opcode, InstType type,  ArenaAllocator *const allocator)
        : SingleInstruction(opcode, type, allocator) {}
    ~InputsInstr() = default;

    virtual size_t GetInputsCount() const = 0;
    virtual Input &GetInput(size_t idx) = 0;
    virtual void SetInput(Input newInput, size_t idx) = 0;
    virtual void ReplaceInput(const Input &oldInput, Input newInput) = 0;
};

template <int InputsNum> class ConstInputsInst : public InputsInstr {
  public:
    ConstInputsInst(Opcode opcode, InstType type, ArenaAllocator *const allocator) : InputsInstr(opcode, type, allocator) {}

    template <typename... T>
    ConstInputsInst(Opcode opcode, InstType type, ArenaAllocator *const allocator, T... inputs)
        : InputsInstr(opcode, type, allocator), inputs_{inputs...}
    {
      for (auto &it : inputs_) {
          if (it.GetInstruction()) {
              it->AddUser(this);
          }
      }
    }

    size_t GetInputsCount() const override {
      return InputsNum;
    }

    Input &GetInput(size_t idx) override { return inputs_.at(idx); }
    void SetInput(Input newInput, size_t idx) override {
        inputs_.at(idx) = newInput;
        if (newInput.GetInstruction()) {
          newInput->AddUser(this);
        }
    }

    void ReplaceInput(const Input &oldInput, Input newInput) override {
      auto iter = std::find(inputs_.begin(), inputs_.end(), oldInput);
      assert(iter != inputs_.end());
      *iter = newInput;
    }

    std::array<Input, InputsNum> &GetInputs() { return inputs_; }

  private:
    std::array<Input, InputsNum> inputs_;
};

template <> class ConstInputsInst<1> : public InputsInstr {
  public:
    ConstInputsInst(Opcode opcode, InstType type, ArenaAllocator *const allocator) : InputsInstr(opcode, type, allocator) {}
    ConstInputsInst(Opcode opcode, InstType type, Input input, ArenaAllocator *const allocator)
        : InputsInstr(opcode, type, allocator), input_(input)
    {
      	if (input_.GetInstruction()) {
			input_->AddUser(this);
      	}
    }

	size_t GetInputsCount() const override {
        return 1;
    }

    Input &GetInput() { return input_; }
    const Input &GetInput() const { return input_; }
    Input &GetInput(size_t idx) override {
        assert(idx == 0);
        return input_;
    }
    void SetInput(Input newInput, size_t idx) override {
        if (idx <= 0) {
            std::cout << "[SingleInstruction Error] in SetInput" << std::endl;
            std::abort();
        }
        input_ = newInput;
		if (input_.GetInstruction()) {
            input_->AddUser(this);
        }
    }
	void ReplaceInput(const Input &oldInput, Input newInput) override {
        assert(input_ == oldInput);
        input_ = newInput;
    }

  private:
    Input input_;
};

class VarInputsInstr : public InputsInstr {
  public:
    VarInputsInstr(Opcode opcode, InstType type, ArenaAllocator *const allocator) : 
      InputsInstr(opcode, type, allocator), inputs_(allocator->ToSTL()) {
		for (auto &it : inputs_) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
	}

	template <typename Ins>
    VarInputsInstr(Opcode opcode, InstType type, Ins ins, ArenaAllocator *const allocator) : 
      InputsInstr(opcode, type, allocator), inputs_(allocator->ToSTL()) {
		for (auto &it : inputs_) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
	}

	template <typename Ins>
    VarInputsInstr(Opcode opcode, InstType type, std::initializer_list<Ins> ins, ArenaAllocator *const allocator) : 
      InputsInstr(opcode, type, allocator), inputs_(allocator->ToSTL()) {
		for (auto &it : inputs_) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
	}

    ~VarInputsInstr() = default;

    size_t GetInputsCount() const override {
        return inputs_.size();
    }

    Input &GetInput(size_t idx) override { return inputs_.at(idx); }
    void SetInput(Input newInput, size_t idx) override {
        inputs_.at(idx) = newInput;
		if (newInput.GetInstruction()) {
            newInput->AddUser(this);
        }
    }


    void ReplaceInput(const Input &oldInput, Input newInput) override {
        auto iter = std::find(inputs_.begin(), inputs_.end(), oldInput);
        assert(iter != inputs_.end());
        *iter = newInput;
    }

    memory::ArenaVector<Input> &GetInputs() { return inputs_; }
    void AddInput(Input newInput) { 
		inputs_.push_back(newInput);
		if (newInput.GetInstruction()) {
            newInput->AddUser(this);
        }
	}

  protected:
    memory::ArenaVector<Input> inputs_;
};

// Who contains results-----------------------------------------------------
template <typename T> class DestIsImm {
  public:
    DestIsImm(T value) : destImm_(value) {}
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

class ConstInstr : public SingleInstruction, public DestIsImm<uint64_t> {
  public:
    ConstInstr(Opcode opcode, InstType type, ArenaAllocator *const allocator)
        : SingleInstruction(opcode, type, allocator), DestIsImm<uint64_t>(0) {}
    ConstInstr(Opcode opcode, InstType type, uint64_t value, ArenaAllocator *const allocator)
        : SingleInstruction(opcode, type, allocator), DestIsImm<uint64_t>(value) {}
};

class UnaryRegInstr : public ConstInputsInst<1> {
  public:
   UnaryRegInstr(Opcode opcode, InstType type, Input input, ArenaAllocator *const allocator)
        : ConstInputsInst(opcode, type, input, allocator) {}
};

class BinaryRegInstr : public ConstInputsInst<2> {
  public:
    BinaryRegInstr(Opcode opcode, InstType type, Input input1, Input input2, ArenaAllocator *const allocator)
        : ConstInputsInst(opcode, type, allocator, input1, input2) {}
};

class BinaryImmInstr : public ConstInputsInst<1>, public DestIsImm<uint64_t> {
	public:
	BinaryImmInstr(Opcode opcode, InstType type, Input input, uint64_t imm, ArenaAllocator *const allocator)
		  : ConstInputsInst(opcode, type, input, allocator), DestIsImm<uint64_t>(imm) {}
  };

class CompInstr : public ConstInputsInst<2>, public DestCondition {
  public:
    CompInstr(Opcode opcode, InstType type, Conditions ccode, Input in1,
              Input in2, ArenaAllocator *const allocator)
        : ConstInputsInst(opcode, type, allocator, in1, in2), DestCondition(ccode) {}
};

class CastInstr : public ConstInputsInst<1> {
  public:
    CastInstr(InstType fromType, InstType toType, Input input, ArenaAllocator *const allocator)
        : ConstInputsInst(Opcode::CAST, fromType, input, allocator), toType_(toType) {}

    auto GetTargetType() const { return toType_; }
    void SetTargetType(InstType newType) { toType_ = newType; }

  private:
    InstType toType_;
};

class JumpInstr : public SingleInstruction, public DestIsImm<uint64_t> {
  public:
    JumpInstr(Opcode opcode, uint64_t imm, ArenaAllocator *const allocator)
        : SingleInstruction(opcode, InstType::i64, allocator), DestIsImm<uint64_t>(imm) {}
};

class RetInstr : public ConstInputsInst<1> {
  public:
    RetInstr(InstType type, Input input, ArenaAllocator *const allocator)
        : ConstInputsInst<1>(Opcode::RET, type, input, allocator) {}
};

class PhiInstr : public VarInputsInstr {
  public:

    PhiInstr(InstType type, ArenaAllocator *const allocator)
        : VarInputsInstr(Opcode::PHI, type, allocator), sourceBBs_(allocator->ToSTL()) {}

    template <typename Ins, typename Sources>
    PhiInstr(InstType type, Ins input, Sources sources, ArenaAllocator *const allocator)
        : VarInputsInstr(Opcode::PHI, type, input, allocator),
		sourceBBs_(sources.cbegin(), sources.cend(), allocator->ToSTL())
    {
        assert(inputs_.size() == sourceBBs_.size());
    }

    template <typename Ins, typename Sources>
    PhiInstr(InstType type, std::initializer_list<Ins> input, std::initializer_list<Sources> sources,
                   ArenaAllocator *const allocator) : VarInputsInstr(Opcode::PHI, type, input, allocator),
				   sourceBBs_(sources.begin(), sources.end(), allocator->ToSTL())
    {
        assert(inputs_.size() == sourceBBs_.size());
    }

    BB *GetSourceBasicBlock(size_t idx) {
        return sourceBBs_.at(idx);
    }
    const BB *GetSourceBasicBlock(size_t idx) const {
        return sourceBBs_.at(idx);
    }
    void SetSourceBasicBlock(BB *bblock, size_t idx) {
        assert(bblock);
        sourceBBs_.at(idx) = bblock;
    }

private:
    memory::ArenaVector<BB *> sourceBBs_;
};

class InputArgInstr : public SingleInstruction {
  public:
    explicit InputArgInstr(InstType type, ArenaAllocator *const allocator)
        : SingleInstruction(Opcode::ARG, type, allocator) {}
};

// ------------------------------------------------------------------------------------------

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_INSTRUCTIONS_H_

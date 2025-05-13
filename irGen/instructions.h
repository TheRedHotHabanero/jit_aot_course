/*
    Instruction structure is stolen from lecture

                                                   Basic Block
    ____________________________________________________________________________________________________________________
    | Instruction 1                Instruction 2 Instruction n   | | - prev -
   prev           (intrusive linked list)                             - prev |
    | - next                       - next           (intrusive linked list) | |
   - next          | | - opcode           <-->      - opcode         (common
   part)              <---> | <---> | <--->    - opcode        | | - type - type
   (common part)                    |       |          - type          | | -
   basic block                - basic block    (common part) - basic block   |
    | - inputs                     - inputs         (derived part) - inputs |
    ____________________________________________________________________________________________________________________|



    Id              - uniq number of inst. If phi -> id contains "p"
    Type            - type of result of inst (s32, u64, ...)
    Opcode          - name of instruction
    Basic flags     - basic inst properties (throwable, no_dce, ...)
    Special flags   - special properties for inst (conditional code, inlined,
   ...) Inputs          - array of instructions, which need to execute the
   instruction Users           - list of instructions, which have instuction as
   input
*/

#ifndef JIT_AOT_COURSE_IR_GEN_INSTRUCTIONS_H_
#define JIT_AOT_COURSE_IR_GEN_INSTRUCTIONS_H_

#include "domTree/arena.h"
#include "input.h"
#include "singleInstruction.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <vector>
namespace ir {
class BB;
using memory::ArenaAllocator;

enum class Conditions { EQ, NONEQ, LSTHAN, GRTHAN };

class InputsInstr : public SingleInstruction {
  public:
    InputsInstr(Opcode opcode, InstType type, ArenaAllocator *const allocator)
        : SingleInstruction(opcode, type, allocator) {}
    ~InputsInstr() = default;

    virtual size_t GetInputsCount() const = 0;
    virtual Input &GetInput(size_t idx) = 0;
    virtual void SetInput(Input newInput, size_t idx) = 0;
    virtual void ReplaceInput(const Input &oldInput, Input newInput) = 0;
};

template <int InputsNum> class ConstInputsInst : public InputsInstr {
  public:
    ConstInputsInst(Opcode opcode, InstType type,
                    ArenaAllocator *const allocator)
        : InputsInstr(opcode, type, allocator) {}

    template <typename... T>
    ConstInputsInst(Opcode opcode, InstType type,
                    ArenaAllocator *const allocator, T... inputs)
        : InputsInstr(opcode, type, allocator), inputs_{inputs...} {
        for (auto &it : inputs_) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
    }

    size_t GetInputsCount() const override {
        return inputs_.size();
        ;
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
    ConstInputsInst(Opcode opcode, InstType type,
                    ArenaAllocator *const allocator)
        : InputsInstr(opcode, type, allocator) {}
    ConstInputsInst(Opcode opcode, InstType type, Input input,
                    ArenaAllocator *const allocator)
        : InputsInstr(opcode, type, allocator), input_(input) {
        if (input_.GetInstruction()) {
            input_->AddUser(this);
        }
    }

    size_t GetInputsCount() const override { return 1; }

    Input &GetInput() { return input_; }
    Input &GetInput([[maybe_unused]] size_t idx) override { return input_; }
    void SetInput(Input newInput, size_t idx) override {
        if (idx != 0) {
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
    VarInputsInstr(Opcode opcode, InstType type,
                   ArenaAllocator *const allocator)
        : InputsInstr(opcode, type, allocator), inputs_(allocator->ToSTL()) {}

    template <typename Ins>
    VarInputsInstr(Opcode opcode, InstType type, Ins ins,
                   ArenaAllocator *const allocator)
        : InputsInstr(opcode, type, allocator),
          inputs_(ins.begin(), ins.end(), allocator->ToSTL()) {
        for (auto &it : inputs_) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
    }

    template <typename Ins>
    VarInputsInstr(Opcode opcode, InstType type, std::initializer_list<Ins> ins,
                   ArenaAllocator *const allocator)
        : InputsInstr(opcode, type, allocator),
          inputs_(ins.begin(), ins.end(), allocator->ToSTL()) {
        for (auto &it : inputs_) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
    }

    template <typename Ins, typename AllocatorT>
    VarInputsInstr(Opcode opcode, InstType type,
                   std::vector<Ins, AllocatorT> ins,
                   ArenaAllocator *const allocator)
        : InputsInstr(opcode, type, allocator),
          inputs_(ins.begin(), ins.end(), allocator->ToSTL()) {
        for (auto &it : inputs_) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
    }

    ~VarInputsInstr() = default;

    size_t GetInputsCount() const override { return inputs_.size(); }

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

class DestTypeId {
  public:
    explicit DestTypeId(TypeId type) : typeId_(type) {}

    auto GetTypeId() const { return typeId_; }
    void SetTypeId(TypeId newType) { typeId_ = newType; }

  private:
    uint64_t typeId_;
};
// ----------------------------------------------------------------------------------

// Specific Instruction containers according to Opcode

class ConstInstr : public SingleInstruction, public DestIsImm<uint64_t> {
  public:
    ConstInstr(Opcode opcode, InstType type, ArenaAllocator *const allocator)
        : SingleInstruction(opcode, type, allocator), DestIsImm<uint64_t>(0) {}
    ConstInstr(Opcode opcode, InstType type, uint64_t value,
               ArenaAllocator *const allocator)
        : SingleInstruction(opcode, type, allocator), DestIsImm<uint64_t>(
                                                          value) {}
    ConstInstr *Copy(BB *targetBBlock) override;
};

class UnaryRegInstr : public ConstInputsInst<1> {
  public:
    UnaryRegInstr(Opcode opcode, InstType type, Input input,
                  ArenaAllocator *const allocator)
        : ConstInputsInst(opcode, type, input, allocator) {}
    UnaryRegInstr *Copy(BB *targetBBlock) override;
};

class BinaryRegInstr : public ConstInputsInst<2> {
  public:
    BinaryRegInstr(Opcode opcode, InstType type, Input input1, Input input2,
                   ArenaAllocator *const allocator)
        : ConstInputsInst(opcode, type, allocator, input1, input2) {}
    BinaryRegInstr *Copy(BB *targetBBlock) override;
};

class BinaryImmInstr : public ConstInputsInst<1>, public DestIsImm<uint64_t> {
  public:
    BinaryImmInstr(Opcode opcode, InstType type, Input input, uint64_t imm,
                   ArenaAllocator *const allocator)
        : ConstInputsInst(opcode, type, input, allocator), DestIsImm<uint64_t>(
                                                               imm) {}
    BinaryImmInstr *Copy(BB *targetBBlock) override;
};

class CompInstr : public ConstInputsInst<2>, public DestCondition {
  public:
    CompInstr(Opcode opcode, InstType type, Conditions ccode, Input in1,
              Input in2, ArenaAllocator *const allocator)
        : ConstInputsInst(opcode, type, allocator, in1, in2),
          DestCondition(ccode) {}
    CompInstr *Copy(BB *targetBBlock) override;
};

class CastInstr : public ConstInputsInst<1> {
  public:
    CastInstr(InstType fromType, InstType toType, Input input,
              ArenaAllocator *const allocator)
        : ConstInputsInst(Opcode::CAST, fromType, input, allocator),
          toType_(toType) {}

    auto GetTargetType() const { return toType_; }
    void SetTargetType(InstType newType) { toType_ = newType; }
    CastInstr *Copy(BB *targetBBlock) override;

  private:
    InstType toType_;
};

class JumpInstr : public SingleInstruction {
  public:
    JumpInstr(Opcode opcode, ArenaAllocator *const allocator)
        : SingleInstruction(opcode, InstType::i64, allocator, INVALID_ID,
                            static_cast<uint8_t>(InstrProp::JUMP)) {}
    JumpInstr *Copy(BB *targetBBlock) override;
    BB *GetDestination();
};

class CondJumpInstr : public SingleInstruction {
  public:
    CondJumpInstr(ArenaAllocator *const allocator)
        : SingleInstruction(Opcode::JCMP, InstType::i64, allocator, INVALID_ID,
                            static_cast<uint8_t>(InstrProp::JUMP) ||
                                static_cast<uint8_t>(InstrProp::SIDE_EFFECTS)) {
    }

    BB *GetTrueDestination();

    BB *GetFalseDestination();
    CondJumpInstr *Copy(BB *targetBBlock) override;

  private:
    template <int CmpRes> BB *GetBranchDestinationImpl();
};

class RetInstr : public ConstInputsInst<1> {
  public:
    RetInstr(InstType type, Input input, ArenaAllocator *const allocator)
        : ConstInputsInst<1>(Opcode::RET, type, input, allocator) {}
    RetInstr *Copy(BB *targetBBlock) override;
};

class RetVoidInstr : public SingleInstruction {
  public:
    RetVoidInstr(ArenaAllocator *const allocator)
        : SingleInstruction(Opcode::RETVOID, InstType::VOID, allocator,
                            INVALID_ID,
                            static_cast<uint8_t>(InstrProp::JUMP) ||
                                static_cast<uint8_t>(InstrProp::SIDE_EFFECTS)) {
    }
    RetVoidInstr *Copy(BB *targetBBlock) override;
};

class PhiInstr : public VarInputsInstr {
  public:
    PhiInstr(InstType type, ArenaAllocator *const allocator)
        : VarInputsInstr(Opcode::PHI, type, allocator),
          sourceBBs_(allocator->ToSTL()) {}

    template <typename Ins, typename Sources>
    PhiInstr(InstType type, Ins input, Sources sources,
             ArenaAllocator *const allocator)
        : VarInputsInstr(Opcode::PHI, type, input, allocator),
          sourceBBs_(sources.cbegin(), sources.cend(), allocator->ToSTL()) {
        assert(inputs_.size() == sourceBBs_.size());
    }

    template <typename Ins, typename Sources>
    PhiInstr(InstType type, std::initializer_list<Ins> input,
             std::initializer_list<Sources> sources,
             ArenaAllocator *const allocator)
        : VarInputsInstr(Opcode::PHI, type, input, allocator),
          sourceBBs_(sources.begin(), sources.end(), allocator->ToSTL()) {
        assert(inputs_.size() == sourceBBs_.size());
    }

    memory::ArenaVector<BB *> &GetSourceBBs() { return sourceBBs_; }

    BB *GetSourceBB(size_t idx) { return sourceBBs_.at(idx); }

    void SetSourceBB(BB *bblock, size_t idx) {
        assert(bblock);
        sourceBBs_.at(idx) = bblock;
    }

    void AddPhiInput(Input newInput, BB *inputSource) {
        assert(inputSource);
        AddInput(newInput);
        sourceBBs_.push_back(inputSource);
    }

    PhiInstr *Copy(BB *targetBBlock) override;

  private:
    memory::ArenaVector<BB *> sourceBBs_;
};

class InputArgInstr : public SingleInstruction {
  public:
    explicit InputArgInstr(InstType type, ArenaAllocator *const allocator)
        : SingleInstruction(Opcode::ARG, type, allocator) {}
    InputArgInstr *Copy(BB *targetBBlock) override;
};

class CallInstr : public VarInputsInstr {
  public:
    CallInstr(InstType type, FunctionID target, ArenaAllocator *const allocator)
        : VarInputsInstr(Opcode::CALL, type, allocator), callTarget_(target),
          isInlined_(false) {}

    template <typename Ins>
    CallInstr(InstType type, FunctionID target, Ins input,
              ArenaAllocator *const allocator)
        : VarInputsInstr(Opcode::CALL, type, input, allocator),
          callTarget_(target) {}

    FunctionID GetCallTarget() const { return callTarget_; }

    void SetCallTarget(FunctionID newTarget) { callTarget_ = newTarget; }
    bool IsInlined() const { return isInlined_; }
    void SetIsInlined(bool inlined) { isInlined_ = inlined; }

    CallInstr *Copy(BB *targetBBlock) override;

  private:
    FunctionID callTarget_;
    bool isInlined_;
};

class LengthInstr : public UnaryRegInstr {
  public:
    LengthInstr(Input array, ArenaAllocator *const allocator)
        : UnaryRegInstr(Opcode::LEN, InstType::u64, array, allocator) {
        assert(!array.GetInstruction() || array->GetType() == InstType::REF);
    }

    LengthInstr *Copy(BB *targetBBlock) override;
};

class NewArrayInstr : public ConstInputsInst<1>, public DestTypeId {
  public:
    NewArrayInstr(Input length, TypeId typeId, ArenaAllocator *const allocator)
        : ConstInputsInst<1>(Opcode::NEW_ARRAY, InstType::REF, length,
                             allocator),
          DestTypeId(typeId) {
        assert(!length.GetInstruction() || IsIntegerType(length->GetType()));
    }

    NewArrayInstr *Copy(BB *targetBBlock) override;
};

class NewArrayImmInstr : public SingleInstruction,
                         public DestIsImm<uint64_t>,
                         public DestTypeId {
  public:
    NewArrayImmInstr(uint64_t length, TypeId typeId,
                     ArenaAllocator *const allocator)
        : SingleInstruction(Opcode::NEW_ARRAY_IMM, InstType::REF, allocator),
          DestIsImm<uint64_t>(length), DestTypeId(typeId) {
        assert(length > 0);
    }

    NewArrayImmInstr *Copy(BB *targetBBlock) override;
};

class NewObjectInstr : public SingleInstruction, public DestTypeId {
  public:
    NewObjectInstr(TypeId typeId, ArenaAllocator *const allocator)
        : SingleInstruction(Opcode::NEW_OBJECT, InstType::REF, allocator),
          DestTypeId(typeId) {}

    NewObjectInstr *Copy(BB *targetBBlock) override;
};

class LoadArrayInstr : public BinaryRegInstr {
  public:
    LoadArrayInstr(InstType type, Input array, Input idx,
                   ArenaAllocator *const allocator)
        : BinaryRegInstr(Opcode::LOAD_ARRAY, type, array, idx, allocator) {
        assert(!array.GetInstruction() || array->GetType() == InstType::REF);
        assert(!idx.GetInstruction() || IsIntegerType(idx->GetType()));
    }

    LoadArrayInstr *Copy(BB *targetBBlock) override;
};

class LoadImmInstr : public BinaryImmInstr {
  public:
    LoadImmInstr(Opcode opcode, InstType type, Input obj, uint64_t offset,
                 ArenaAllocator *const allocator)
        : BinaryImmInstr(opcode, type, obj, offset, allocator) {
        assert(opcode == Opcode::LOAD_ARRAY_IMM ||
               opcode == Opcode::LOAD_OBJECT);
        assert(!obj.GetInstruction() || obj->GetType() == InstType::REF);
    }

    LoadImmInstr *Copy(BB *targetBBlock) override;
};

class StoreArrayInstr : public ConstInputsInst<3> {
  public:
    StoreArrayInstr(Input array, Input storedValue, Input idx,
                    ArenaAllocator *const allocator)
        : ConstInputsInst<3>(Opcode::STORE_ARRAY, InstType::VOID, allocator,
                             array, storedValue, idx) {
        assert(!array.GetInstruction() || array->GetType() == InstType::REF);
        assert(!idx.GetInstruction() || IsIntegerType(idx->GetType()));
    }

    StoreArrayInstr *Copy(BB *targetBBlock) override;
};

class StoreImmInstr : public ConstInputsInst<2>, public DestIsImm<uint64_t> {
  public:
    StoreImmInstr(Opcode opcode, Input obj, Input storedValue, uint64_t offset,
                  ArenaAllocator *const allocator)
        : ConstInputsInst<2>(opcode, InstType::VOID, allocator, obj,
                             storedValue),
          DestIsImm<uint64_t>(offset) {
        assert(opcode == Opcode::STORE_ARRAY_IMM ||
               opcode == Opcode::STORE_OBJECT);
        assert(!obj.GetInstruction() || obj->GetType() == InstType::REF);
    }

    StoreImmInstr *Copy(BB *targetBBlock) override;
};

class BoundsCheckInstr : public ConstInputsInst<2> {
  public:
    BoundsCheckInstr(Input arr, Input idx, ArenaAllocator *const allocator)
        : ConstInputsInst<2>(Opcode::BOUNDS_CHECK, InstType::INVALID, allocator,
                             arr, idx) {
        assert(!arr.GetInstruction() || arr->GetType() == InstType::REF);
        assert(!idx.GetInstruction() || IsIntegerType(idx->GetType()));
    }

    BoundsCheckInstr *Copy(BB *targetBBlock) override;
};

// ------------------------------------------------------------------------------------------

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_INSTRUCTIONS_H_

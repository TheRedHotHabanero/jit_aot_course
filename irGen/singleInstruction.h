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

#ifndef JIT_AOT_COURSE_IR_GEN_SINGLE_INSTRUCTION_H_
#define JIT_AOT_COURSE_IR_GEN_SINGLE_INSTRUCTION_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include "input.h"
namespace ir {
class BB;

enum class InstType { i8, i16, i32, i64, u8, u16, u32, u64, VOID, INVALID };

enum class Opcode {
    MUL,
    MULI,
    SHR,
    SHRI,
    XOR,
    XORI,
    ADDI,
    CAST, // Type1 as type2
    CMP,
    JA,  // Cond jump
    JMP, // Non-cond jump
    RET,
    PHI,
    CONST,
    ARG,
    INVALID
};

static constexpr std::array<const char *,
                            static_cast<size_t>(Opcode::INVALID) + 1>
    nameOpcode{"MUL", "MULI", "SHR", "SHRI", "XOR", "XORI", "ADDI", "MOVI", "CAST",   "CMP",
               "JA",  "JMP",  "RET",  "INVALID"};

class ConstInstr;
class SingleInstruction {
  public:
    SingleInstruction(Opcode opcode, InstType type)
        : opcode_(opcode), prevInst_(nullptr), nextInst_(nullptr),
          instBB_(nullptr), instType_(type), instID_(INVALID_ID) {}
    SingleInstruction(const SingleInstruction &) = delete;
    SingleInstruction &operator=(const SingleInstruction &) = delete;
    SingleInstruction(SingleInstruction &&) = delete;
    SingleInstruction &operator=(SingleInstruction &&) = delete;
    virtual ~SingleInstruction() = default;

  private:
    Opcode opcode_;
    SingleInstruction *prevInst_;
    SingleInstruction *nextInst_;
    BB *instBB_;
    InstType instType_;
    size_t instID_;
    std::vector<Input *> inputs_;

  public:
    // getters
    size_t GetInstID() const { return instID_; }
    SingleInstruction *GetPrevInst() { return prevInst_; }
    SingleInstruction *GetNextInst() { return nextInst_; }
    ConstInstr *CastToConstant();
    BB *GetInstBB() const { return instBB_; }
    Opcode GetOpcode() const { return opcode_; }
    const char *GetOpcodeName(Opcode opcode) const;
    auto GetRegType() { return instType_; }
    bool IsInputArgument() const { return opcode_ == Opcode::ARG; }
    bool IsPhi() const { return opcode_ == Opcode::PHI; }
    static const size_t INVALID_ID = static_cast<size_t>(0) - 1;
    Input *GetInput(size_t idx) { return inputs_.at(idx); }

    void SetInput(Input *newInput, size_t idx) {
        inputs_.at(idx) = newInput;
    }
    
    std::vector<Input *> &GetInputs() { return inputs_; }
    bool IsConst() const {
      return GetOpcode() == Opcode::CONST;
    }

    void ReplaceWith(SingleInstruction *new_inst) {
        if (prevInst_) {
            prevInst_->SetNextInst(new_inst);
        }

        if (nextInst_) {
            nextInst_->SetPrevInst(new_inst);
        }
    
        new_inst->SetPrevInst(prevInst_);
        new_inst->SetNextInst(nextInst_);
        
        new_inst->SetBB(instBB_);

        RemoveFromBlock();
    }
    

  public:
    // setters
    void SetInstId(size_t newID) { instID_ = newID; }
    void SetPrevInst(SingleInstruction *inst) { prevInst_ = inst; }
    void SetNextInst(SingleInstruction *inst) { nextInst_ = inst; }
    void SetBB(BB *bb) { instBB_ = bb; }
    void RemoveFromBlock();
    void InsertInstBefore(SingleInstruction *inst);
    void InsertInstAfter(SingleInstruction *inst);
    void SetRegType(InstType type) { instType_ = type; }
    void PrintSSA();
};

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_SINGLE_INSTRUCTION_H_

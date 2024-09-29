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

#include <array>
#include <cstddef>
#include <cstdint>
namespace ir {
class BB;
// Type - type of result of inst (s32, u64, ...)
    // TODO: void
enum class InstType { i8, i16, i32, i64, u8, u16, u32, u64, INVALID };

enum class Opcode {
    MUL,
    ADDI,
    MOVI, // Moving constant
    CAST, // Type1 as type2
    CMP,
    JA,  // Cond jump
    JMP, // Non-cond jump
    RET,
    PHI,
    INVALID
};

static constexpr std::array<const char *,
                            static_cast<size_t>(Opcode::INVALID) + 1>
    nameOpcode{"MUL", "ADDI", "MOVI", "CAST",   "CMP",
               "JA",  "JMP",  "RET",  "INVALID"};

enum class Conditions { EQ, NONEQ, LSTHAN, GRTHAN };

class VReg {
  public:
    explicit VReg(uint8_t value) : value_(value) {}
    uint8_t GetRegValue() { return value_; }
    VReg() = default;
    VReg(const VReg &) = default;
    VReg &operator=(const VReg &) = default;
    VReg(VReg &&) = default;
    VReg &operator=(VReg &&) = default;

  private:
    uint8_t value_;
};

// bool operator==(VReg &lhs, VReg &rhs) {
//     return lhs.GetRegValue() == rhs.GetRegValue();
// }

class SingleInstruction {
  public:
    SingleInstruction(Opcode opcode, InstType type)
        : opcode_(opcode), prevInst_(nullptr), nextInst_(nullptr),
          instBB_(nullptr), instType_(type) {}
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
    VReg vreg_;
    VReg vreg1_;
    VReg vreg2_;

  public:
    // getters
    SingleInstruction *GetPrevInst() { return prevInst_; }
    SingleInstruction *GetNextInst() { return nextInst_; }
    BB *GetInstBB() const { return instBB_; }
    Opcode GetOpcode() const { return opcode_; }
    const char *GetOpcodeName(Opcode opcode) const;
    auto GetRegType() { return instType_; }
    VReg GetVirtualReg() { return vreg_; }
    VReg GetVirtualReg1() { return vreg1_; }
    VReg GetVirtualReg2() { return vreg2_; }

  public:
    // setters
    void SetPrevInst(SingleInstruction *inst) { prevInst_ = inst; }
    void SetNextInst(SingleInstruction *inst) { nextInst_ = inst; }
    void SetBB(BB *bb) { instBB_ = bb; }
    void RemoveFromBlock();
    void InsertInstBefore(SingleInstruction *inst);
    void InsertInstAfter(SingleInstruction *inst);
    void SetRegType(InstType type) { instType_ = type; }
    void SetVirtualReg(VReg vreg) { vreg_ = vreg; }
    void SetVirtualReg1(VReg vreg) { vreg1_ = vreg; }
    void SetVirtualReg2(VReg vreg) { vreg2_ = vreg; }
    void PrintSSA();
};

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_INSTRUCTIONS_H_

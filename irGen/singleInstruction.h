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

#ifndef JIT_AOT_COURSE_IR_GEN_SINGLE_INSTRUCTION_H_
#define JIT_AOT_COURSE_IR_GEN_SINGLE_INSTRUCTION_H_

#include "domTree/arena.h"
#include "input.h"
#include "marker.h"
#include "user.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>
namespace ir {
using memory::ArenaAllocator;
using memory::ArenaVector;
class BB;

using FunctionID = size_t;
enum class InstType {
    i8,
    i16,
    i32,
    i64,
    u8,
    u16,
    u32,
    u64,
    VOID,
    REF,
    INVALID
};

constexpr inline int64_t ToSigned(uint64_t value, InstType type) {
    switch (type) {
    case InstType::u8:
        return static_cast<int64_t>(static_cast<int8_t>(value));
    case InstType::u16:
        return static_cast<int64_t>(static_cast<int16_t>(value));
    case InstType::u32:
        return static_cast<int64_t>(static_cast<int32_t>(value));
    case InstType::u64:
        return static_cast<int64_t>(static_cast<int64_t>(value));
    default:
        return static_cast<int64_t>(value);
    }
}

constexpr std::array<uint64_t, static_cast<size_t>(InstType::INVALID)>
    maxValues{0,
              std::numeric_limits<int8_t>::max(),
              std::numeric_limits<int16_t>::max(),
              std::numeric_limits<int32_t>::max(),
              std::numeric_limits<int64_t>::max(),
              std::numeric_limits<uint8_t>::max(),
              std::numeric_limits<uint16_t>::max(),
              std::numeric_limits<uint32_t>::max(),
              std::numeric_limits<uint64_t>::max()};

constexpr inline uint64_t GetMaxValue(InstType type) {
    assert(type != InstType::INVALID);
    return maxValues[static_cast<size_t>(type)];
}

constexpr inline bool IsIntegerType(InstType type) {
    auto t = static_cast<uint8_t>(type);
    return static_cast<uint8_t>(InstType::i8) <= t &&
           t <= static_cast<uint8_t>(InstType::u64);
}

class TypeId {
  public:
    TypeId(uint64_t id) : id(id) {}
    TypeId(const TypeId &) = default;
    TypeId &operator=(const TypeId &) = default;
    TypeId(TypeId &&) = default;
    TypeId &operator=(TypeId &&) = default;
    virtual ~TypeId() noexcept = default;

    operator uint64_t() const { return id; }

  private:
    uint64_t id;
};

enum class Opcode {
    MUL,
    MULI,
    SHR,
    SHRI,
    XOR,
    XORI,
    ADDI,
    ADD,
    CAST,
    CMP,
    JMP,
    JCMP,
    RET,
    RETVOID,
    CALL,
    PHI,
    CONST,
    ARG,
    LOAD,
    STORE,
    LEN,
    NEW_ARRAY,
    NEW_ARRAY_IMM,
    NEW_OBJECT,
    LOAD_ARRAY,
    LOAD_ARRAY_IMM,
    LOAD_OBJECT,
    STORE_ARRAY,
    STORE_ARRAY_IMM,
    STORE_OBJECT,
    BOUNDS_CHECK,
    NULL_CHECK,
    INVALID
};

static constexpr std::array<const char *,
                            static_cast<size_t>(Opcode::INVALID) + 1>
    nameOpcode{"MUL",  "MULI", "SHR", "SHRI", "XOR", "XORI", "ADDI",
               "MOVI", "CAST", "CMP", "JA",   "JMP", "RET",  "INVALID"};

// Instructions properties, used in optimizations
enum class InstrProp : uint8_t {
    ARITH = 0b1,
    MEM = 0b10,
    COMMUTABLE = 0b100,
    JUMP = 0b1000,
    INPUT = 0b10000,
    SIDE_EFFECTS = 0b100000,
};

constexpr inline uint8_t operator|(InstrProp lhs, InstrProp rhs) {
    return static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs);
}

constexpr inline uint8_t operator|(uint8_t lhs, InstrProp rhs) {
    return lhs | static_cast<uint8_t>(rhs);
}

constexpr inline uint8_t operator|(InstrProp lhs, uint8_t rhs) {
    return static_cast<uint8_t>(lhs) | rhs;
}

template <typename T>
constexpr inline uint8_t &operator|=(uint8_t &lhs, T rhs) {
    lhs = lhs | rhs;
    return lhs;
}

class ConstInstr;
class InputsInstr;
class SingleInstruction : public Markable, public User {
  public:
    SingleInstruction(Opcode opcode, InstType type,
                      ArenaAllocator *const allocator, size_t id = INVALID_ID,
                      uint8_t prop = 0)
        : User(allocator), opcode_(opcode), instType_(type), instID_(id),
          properties_(prop) {}
    SingleInstruction(const SingleInstruction &) = delete;
    SingleInstruction &operator=(const SingleInstruction &) = delete;
    SingleInstruction(SingleInstruction &&) = delete;
    SingleInstruction &operator=(SingleInstruction &&) = delete;
    virtual ~SingleInstruction() = default;

  private:
    Opcode opcode_;
    SingleInstruction *prevInst_ = nullptr;
    SingleInstruction *nextInst_ = nullptr;
    BB *instBB_ = nullptr;
    InstType instType_;
    size_t instID_;
    uint8_t properties_ = 0;

  public:
    // getters
    size_t GetInstID() const { return instID_; }
    SingleInstruction *GetPrevInst() { return prevInst_; }
    SingleInstruction *GetNextInst() { return nextInst_; }
    ConstInstr *CastToConstant();
    BB *GetInstBB() const { return instBB_; }
    Opcode GetOpcode() const { return opcode_; }
    const char *GetOpcodeName(Opcode opcode) const;
    auto GetType() { return instType_; }
    uint8_t GetProperties() const { return properties_; }
    bool SatisfiesProperty(InstrProp prop) const {
        return GetProperties() & static_cast<uint8_t>(prop);
    }
    bool IsInputArgument() const { return GetOpcode() == Opcode::ARG; }
    bool IsPhi() const { return GetOpcode() == Opcode::PHI; }
    static constexpr size_t INVALID_ID = static_cast<size_t>(0) - 1;
    bool IsConst() const { return GetOpcode() == Opcode::CONST; }
    bool IsCall() const { return GetOpcode() == Opcode::CALL; }
    bool IsBranch() const { return GetOpcode() == Opcode::JCMP; }
    bool HasSideEffects() const {
        return SatisfiesProperty(InstrProp::SIDE_EFFECTS);
    }
    bool HasInputs() const { return SatisfiesProperty(InstrProp::INPUT); }

    template <typename T> constexpr inline void SetProperty(T prop) {
        properties_ |= prop;
    }
    void SetProperty(uint8_t prop) { properties_ |= prop; }

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

    void ReplaceInputInUsers(SingleInstruction *newInput);

    virtual SingleInstruction *Copy(BB *targetBBlock) = 0;

  public:
    // setters
    bool Dominates(SingleInstruction *other);
    void SetInstId(size_t newID) { instID_ = newID; }
    void SetPrevInst(SingleInstruction *inst) { prevInst_ = inst; }
    void SetNextInst(SingleInstruction *inst) { nextInst_ = inst; }
    void SetBB(BB *bb) { instBB_ = bb; }
    void RemoveFromBlock();
    void InsertInstBefore(SingleInstruction *inst);
    void InsertInstAfter(SingleInstruction *inst);
    void SetRegType(InstType type) { instType_ = type; }
    void PrintSSA();
    bool IsEarlierInBasicBlock(SingleInstruction *other);
};

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_SINGLE_INSTRUCTION_H_

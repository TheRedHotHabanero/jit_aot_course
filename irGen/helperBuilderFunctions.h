#ifndef JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_
#define JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_

#include "domTree/arena.h"
#include "graph.h"
#include "instructions.h"
#include <cstdint>
#include <type_traits>
#include <vector>

namespace ir {
using memory::ArenaAllocator;
using memory::ArenaVector;

class InstructionBuilder {

  private:
    ArenaAllocator *const allocator_;
    ArenaVector<SingleInstruction *> instructions_;
    static constexpr InstructionPropT ARITHM =
        std::to_underlying(InstrProp::ARITH) ||
        std::to_underlying(InstrProp::INPUT);
        static constexpr InstructionPropT SIDE_EFFECTS_ARITHM =
        std::to_underlying(InstrProp::ARITH) || 
        std::to_underlying(InstrProp::INPUT) ||
        std::to_underlying(InstrProp::SIDE_EFFECTS);

  public:
    explicit InstructionBuilder(ArenaAllocator *const allocator)
        : allocator_(allocator), instructions_(allocator_->ToSTL()) {
        assert(allocator_);
    }
    InstructionBuilder(const InstructionBuilder &) = delete;
    InstructionBuilder &operator=(const InstructionBuilder &) = delete;
    InstructionBuilder(InstructionBuilder &&) = delete;
    InstructionBuilder &operator=(InstructionBuilder &&) = delete;
    virtual ~InstructionBuilder() noexcept = default;

    static void PushBackInst(BB *bb, SingleInstruction *instr) {
        bb->PushInstBackward(instr);
    }

    void PushForwardInst(BB *bb, SingleInstruction *instr) {
        bb->PushInstForward(instr);
    }

    void AttachInstruction(SingleInstruction *inst) {
        assert(inst);
        assert(std::find(instructions_.begin(), instructions_.end(), inst) ==
               instructions_.end());
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
    }

    SingleInstruction *GetLastInst() {
        return instructions_[instructions_.size() - 1];
    }

  public:
    template <typename T,
              typename = std::enable_if_t<
                  std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                  std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>>>
    ConstInstr *BuildConst(InstType type, T imm) {
        auto *inst = allocator_->template New<ConstInstr>(Opcode::CONST, type,
                                                          imm, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    CastInstr *BuildCast(InstType fromType, InstType targetType, Input input) {
        auto *inst = allocator_->template New<CastInstr>(fromType, targetType,
                                                         input, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(InstrProp::INPUT);
        return inst;
    }

    CompInstr *BuildCmp(InstType type, Conditions conditions, Input input1,
                        Input input2) {
        auto *inst = allocator_->template New<CompInstr>(
            Opcode::CMP, type, conditions, input1, input2, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        auto prop = std::to_underlying(InstrProp::INPUT) || std::to_underlying(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    JumpInstr *BuildJmp() {
        auto *inst =
            allocator_->template New<JumpInstr>(Opcode::JMP, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(InstrProp::JUMP);
        return inst;
    }

    CondJumpInstr *BuildJcmp() {
        auto *inst = allocator_->template New<CondJumpInstr>(allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    RetInstr *BuildRet(InstType type, Input input) {
        auto *inst =
            allocator_->template New<RetInstr>(type, input, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        auto prop = std::to_underlying(InstrProp::JUMP) ||
                    std::to_underlying(InstrProp::INPUT) ||
                    std::to_underlying(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    RetVoidInstr *BuildRetVoid() {
        auto *inst = allocator_->template New<RetVoidInstr>(allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    CallInstr *BuildCall(InstType type, FunctionID target) {
        auto *inst =
            allocator_->template New<CallInstr>(type, target, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        auto prop = std::to_underlying(InstrProp::INPUT) ||
                    std::to_underlying(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    template <typename Ins>
    CallInstr *BuildCall(InstType type, FunctionID target,
                         std::initializer_list<Ins> args) {
        auto *inst =
            allocator_->template New<CallInstr>(type, target, args, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        auto prop = std::to_underlying(InstrProp::INPUT) ||
                    std::to_underlying(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    template <typename Ins, typename AllocatorT>
    CallInstr *BuildCall(InstType type, FunctionID target,
                         std::vector<Ins, AllocatorT> args) {
        auto *inst =
            allocator_->template New<CallInstr>(type, target, args, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        auto prop = std::to_underlying(InstrProp::INPUT) ||
                    std::to_underlying(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    LoadInstr *BuildLoad(InstType type, uint64_t addr) {
        auto *inst =
            allocator_->template New<LoadInstr>(type, addr, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        auto prop = std::to_underlying(InstrProp::MEM) ||
                    std::to_underlying(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    StoreInstr *BuildStore(Input storedValue, uint64_t addr) {
        auto *inst =
            allocator_->template New<StoreInstr>(storedValue, addr, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        auto prop = std::to_underlying(InstrProp::MEM) ||
                    std::to_underlying(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    PhiInstr *BuildPhi(InstType type) {
        auto *inst = allocator_->template New<PhiInstr>(type, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(InstrProp::INPUT);
        return inst;
    }

    template <typename Ins, typename Sources>
    PhiInstr *BuildPhi(InstType type, std::initializer_list<Ins> inputs,
                       std::initializer_list<Sources> sources) {
        auto *inst = allocator_->template New<PhiInstr>(type, inputs, sources,
                                                        allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(InstrProp::INPUT);
        return inst;
    }

    template <typename Ins, typename Sources>
    PhiInstr *BuildPhi(InstType type, Ins inputs, Sources sources) {
        auto *inst = allocator_->template New<PhiInstr>(type, inputs, sources,
                                                        allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(InstrProp::INPUT);
        return inst;
    }

    InputArgInstr *BuildArg(InstType type) {
        auto *inst = allocator_->template New<InputArgInstr>(type, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        return inst;
    }

    BinaryRegInstr *BuildShr(InstType type, Input input1, Input input2) {
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::SHR, type, input1, input2, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(ARITHM);
        return inst;
    }

    BinaryRegInstr *BuildXor(InstType type, Input input1, Input input2) {
        auto prop = ARITHM | std::to_underlying(InstrProp::COMMUTABLE);
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::XOR, type, input1, input2, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(prop);
        return inst;
    }

    BinaryRegInstr *BuildMul(InstType type, Input input1, Input input2) {
        auto prop = ARITHM | std::to_underlying(InstrProp::COMMUTABLE);
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::MUL, type, input1, input2, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(prop);
        return inst;
    }

    BinaryRegInstr *BuildAdd(InstType type, Input input1, Input input2) {
        auto prop = ARITHM | std::to_underlying(InstrProp::COMMUTABLE);
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::ADD, type, input1, input2, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(prop);
        return inst;
    }

    template <typename T,
              typename = std::enable_if_t<
                  std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                  std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>>>
    BinaryRegInstr *BuildAddi(InstType type, Input input, T immediate) {
        auto prop = ARITHM | std::to_underlying(InstrProp::COMMUTABLE);
        auto *constInstr = new ConstInstr(
            Opcode::CONST, type, static_cast<uint64_t>(immediate), allocator_);
        Input immInput = Input(constInstr);
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::ADDI, type, input, immInput, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(prop);
        return inst;
    }

    template <typename T,
              typename = std::enable_if_t<
                  std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                  std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>>>
    BinaryRegInstr *BuildMuli(InstType type, Input input, T immediate) {
        auto prop = ARITHM | std::to_underlying(InstrProp::COMMUTABLE);
        auto *constInstr = new ConstInstr(
            Opcode::CONST, type, static_cast<uint64_t>(immediate), allocator_);
        Input immInput = Input(constInstr);
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::MULI, type, input, immInput, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(prop);
        return inst;
    }

    template <typename T,
              typename = std::enable_if_t<
                  std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                  std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>>>
    BinaryRegInstr *BuildXori(InstType type, Input input, T immediate) {
        auto *constInstr = new ConstInstr(
            Opcode::CONST, type, static_cast<uint64_t>(immediate), allocator_);
        Input immInput = Input(constInstr);
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::XORI, type, input, immInput, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(ARITHM);
        return inst;
    }

    template <typename T,
              typename = std::enable_if_t<
                  std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                  std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>>>
    BinaryRegInstr *BuildShri(InstType type, Input input, T immediate) {
        auto *constInstr = new ConstInstr(
            Opcode::CONST, type, static_cast<uint64_t>(immediate), allocator_);
        Input immInput = Input(constInstr);
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::SHRI, type, input, immInput, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(ARITHM);
        return inst;
    }
};
} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_HELPER_BUILDER_FUNCTIONS_H_

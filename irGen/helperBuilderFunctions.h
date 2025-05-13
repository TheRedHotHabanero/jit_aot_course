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
    static constexpr uint8_t ARITHM = static_cast<uint8_t>(InstrProp::ARITH) ||
                                      static_cast<uint8_t>(InstrProp::INPUT);
    static constexpr uint8_t SIDE_EFFECTS_ARITHM =
        static_cast<uint8_t>(InstrProp::ARITH) ||
        static_cast<uint8_t>(InstrProp::INPUT) ||
        static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);
    static constexpr uint8_t INPUT_MEM =
        static_cast<uint8_t>(InstrProp::INPUT) ||
        static_cast<uint8_t>(InstrProp::MEM) ||
        static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);
    static constexpr uint8_t INPUT_SIDE_EFFECTS =
        static_cast<uint8_t>(InstrProp::INPUT) ||
        static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);

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
    template <typename T> ConstInstr *BuildConst(InstType type, T imm) {
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
        auto prop = static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);
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
        auto prop = static_cast<uint8_t>(InstrProp::JUMP) ||
                    static_cast<uint8_t>(InstrProp::INPUT) ||
                    static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);
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
        auto prop = static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);
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
        auto prop = static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);
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
        auto prop = static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    LengthInstr *BuildLen(Input array) {
        auto *inst = allocator_->template New<LengthInstr>(array, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(INPUT_MEM);
        return inst;
    }

    NewArrayInstr *BuildNewArray(Input length, TypeId typeId) {
        auto *inst =
            allocator_->template New<NewArrayInstr>(length, typeId, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        auto prop = static_cast<uint8_t>(InstrProp::MEM) ||
                    static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    NewArrayImmInstr *BuildNewArrayImm(uint64_t length, TypeId typeId) {
        auto *inst = allocator_->template New<NewArrayImmInstr>(length, typeId,
                                                                allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        auto prop = static_cast<uint8_t>(InstrProp::MEM) ||
                    static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    NewObjectInstr *BuildNewObject(TypeId typeId) {
        auto *inst =
            allocator_->template New<NewObjectInstr>(typeId, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        auto prop = static_cast<uint8_t>(InstrProp::MEM) ||
                    static_cast<uint8_t>(InstrProp::SIDE_EFFECTS);
        inst->SetProperty(prop);
        return inst;
    }

    LoadArrayInstr *BuildLoadArray(InstType type, Input array, Input idx) {
        auto *inst = allocator_->template New<LoadArrayInstr>(type, array, idx,
                                                              allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(INPUT_MEM);
        return inst;
    }

    LoadImmInstr *BuildLoadArrayImm(InstType type, Input array, uint64_t idx) {
        auto *inst = allocator_->template New<LoadImmInstr>(
            Opcode::LOAD_ARRAY_IMM, type, array, idx, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(INPUT_MEM);
        return inst;
    }

    LoadImmInstr *BuildLoadObject(InstType type, Input obj, uint64_t offset) {
        auto *inst = allocator_->template New<LoadImmInstr>(
            Opcode::LOAD_OBJECT, type, obj, offset, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(INPUT_MEM);
        return inst;
    }

    StoreArrayInstr *BuildStoreArray(Input array, Input storedValue,
                                     Input idx) {
        auto *inst = allocator_->template New<StoreArrayInstr>(
            array, storedValue, idx, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(INPUT_MEM);
        return inst;
    }

    StoreImmInstr *BuildStoreArrayImm(Input array, Input storedValue,
                                      uint64_t idx) {
        auto *inst = allocator_->template New<StoreImmInstr>(
            Opcode::STORE_ARRAY_IMM, array, storedValue, idx, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(INPUT_MEM);
        return inst;
    }

    StoreImmInstr *BuildStoreObject(Input obj, Input storedValue,
                                    uint64_t offset) {
        auto *inst = allocator_->template New<StoreImmInstr>(
            Opcode::STORE_OBJECT, obj, storedValue, offset, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(INPUT_MEM);
        return inst;
    }

    UnaryRegInstr *BuildNullCheck(Input input) {
        auto *inst = allocator_->template New<UnaryRegInstr>(
            Opcode::NULL_CHECK, InstType::INVALID, input, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(INPUT_SIDE_EFFECTS);
        return inst;
    }

    BoundsCheckInstr *BuildBoundsCheck(Input arr, Input idx) {
        auto *inst =
            allocator_->template New<BoundsCheckInstr>(arr, idx, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(INPUT_SIDE_EFFECTS);
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
        auto prop = ARITHM | static_cast<uint8_t>(InstrProp::COMMUTABLE);
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::XOR, type, input1, input2, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(prop);
        return inst;
    }

    BinaryRegInstr *BuildMul(InstType type, Input input1, Input input2) {
        auto prop = ARITHM | static_cast<uint8_t>(InstrProp::COMMUTABLE);
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::MUL, type, input1, input2, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(prop);
        return inst;
    }

    BinaryRegInstr *BuildAdd(InstType type, Input input1, Input input2) {
        auto prop = ARITHM | static_cast<uint8_t>(InstrProp::COMMUTABLE);
        auto *inst = allocator_->template New<BinaryRegInstr>(
            Opcode::ADD, type, input1, input2, allocator_);
        instructions_.push_back(inst);
        inst->SetInstId(instructions_.size());
        inst->SetProperty(prop);
        return inst;
    }

    template <typename T>
    BinaryRegInstr *BuildAddi(InstType type, Input input, T immediate) {
        auto prop = ARITHM | static_cast<uint8_t>(InstrProp::COMMUTABLE);
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

    template <typename T>
    BinaryRegInstr *BuildMuli(InstType type, Input input, T immediate) {
        auto prop = ARITHM | static_cast<uint8_t>(InstrProp::COMMUTABLE);
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

    template <typename T>
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

    template <typename T>
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

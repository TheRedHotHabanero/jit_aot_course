#ifndef JIT_AOT_COURSE_CONSTANT_FOLDING_H_
#define JIT_AOT_COURSE_CONSTANT_FOLDING_H_

#include "domTree/arena.h"
#include "bb.h"
#include "graph.h"


namespace ir {
class ConstantFolding {
public:
    ConstantFolding() = default;
    ConstantFolding(const ConstantFolding &) = delete;
    ConstantFolding &operator=(const ConstantFolding &) = delete;

    ConstantFolding(ConstantFolding &&) = delete;
    ConstantFolding &operator=(ConstantFolding &&) = delete;

    virtual ~ConstantFolding() = default;

    virtual bool ProcessMUL(BinaryRegInstr *instr);
    virtual bool ProcessSHR(BinaryRegInstr *instr);
    virtual bool ProcessXOR(BinaryRegInstr *instr);

private:
    static ConstInstr *AsConst(SingleInstruction *instr);
    static InstructionBuilder *GetInstructionBuilder(SingleInstruction *instr);
};
}   // namespace ir

#endif  // JIT_AOT_COURSE_CONSTANT_FOLDING_H_
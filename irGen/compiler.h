#ifndef JIT_AOT_COURSE_IR_GEN_COMPILER
#define JIT_AOT_COURSE_IR_GEN_COMPILER

#include "domTree/arena.h"
#include "graph.h"
#include "helperBuilderFunctions.h"
#include "irGen.h"


class CompilerBase {
public:
    CompilerBase() = default;
    CompilerBase(const CompilerBase &) = delete;
    CompilerBase(CompilerBase &&) = delete;
    virtual ~CompilerBase() = default;
};

class Compiler : public CompilerBase {
public:
    Compiler() : allocator_(), instrBuilder(&allocator_), irGenerator_(&allocator_) {}

    ir::InstructionBuilder &GetInstructionBuilder() {
        return instrBuilder;
    }
    ir::IRGenerator &GetIRGenerator() {
        return irGenerator_;
    }

private:
    memory::ArenaAllocator allocator_;

    ir::InstructionBuilder instrBuilder;
    ir::IRGenerator irGenerator_;
};

#endif // JIT_AOT_COURSE_IR_GEN_COMPILER
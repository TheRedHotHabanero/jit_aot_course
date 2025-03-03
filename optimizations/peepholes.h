#pragma once
#include "../irGen/graph.h"
namespace ir {
class Peepholes {
public:
    Peepholes(Graph *graph):
        graph_(graph) {}
    void Run();

    void AnalysisInst(SingleInstruction *inst);
    void ReplaceInstructionWithInput(SingleInstruction *inst, Input *input);
    void ReplaceInstructionWithConstant(SingleInstruction *inst, int32_t constantValue);
private:
    // Process instruction
    void VisitMul(SingleInstruction *inst);
    void VisitShr(SingleInstruction *inst);
    void VisitXor(SingleInstruction *inst);
    // Cases of optimization
    bool TryOptimizeMul(SingleInstruction *inst);
    bool TryOptimizeShr(SingleInstruction *inst);
    bool TryOptimizeXor(SingleInstruction *inst);
private:
    Graph *graph_;
};
}
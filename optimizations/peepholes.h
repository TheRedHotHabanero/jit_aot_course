
#include "constFolding.h"
#include "domTree/arena.h"
#include "irGen/graph.h"
#include "irGen/singleInstruction.h"
#include "pass.h"
namespace ir {
class Peepholes : public OptimizationPassBase {
  public:
    explicit Peepholes(Graph *graph) : OptimizationPassBase(graph) {
        assert(graph);
    }
    ~Peepholes() noexcept override = default;

    // Cases of optimization
    bool TryOptimizeMul(BinaryRegInstr *inst);
    bool TryOptimizeShr(BinaryRegInstr *inst);
    bool TryOptimizeXor(BinaryRegInstr *inst);

    void VisitMul(SingleInstruction *instr);
    void VisitShr(SingleInstruction *instr);
    void VisitXor(SingleInstruction *instr);

    void Run() override;

  private:
    void ReplaceWithoutNewInstr(BinaryRegInstr *instr,
                                SingleInstruction *replacedInstr);
    ConstantFolding constFolding_;
};
} // namespace ir
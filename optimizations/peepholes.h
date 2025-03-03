
#include "domTree/arena.h"
#include "constFolding.h"
#include "irGen/singleInstruction.h"
#include "irGen/graph.h"
namespace ir {
class Peepholes {
public:
    explicit Peepholes(Graph *graph) : graph_(graph) {
        assert(graph);
    }
    Peepholes(const Peepholes &) = delete;
    Peepholes &operator=(const Peepholes &) = delete;

    Peepholes(Peepholes &&) = delete;
    Peepholes &operator=(Peepholes &&) = delete;

    virtual ~Peepholes() = default;

    // Cases of optimization
    bool TryOptimizeMul(BinaryRegInstr *inst);
    bool TryOptimizeShr(BinaryRegInstr *inst);
    bool TryOptimizeXor(BinaryRegInstr *inst);

    void VisitMul(SingleInstruction *instr);
    void VisitShr(SingleInstruction *instr);
    void VisitXor(SingleInstruction *instr);

    void Run();
private:
    void ReplaceWithoutNewInstr(BinaryRegInstr *instr, SingleInstruction *replacedInstr);
    Graph *graph_;
    ConstantFolding constFolding_;
};
}
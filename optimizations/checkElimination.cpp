#include "checkElimination.h"
#include "domTree/dfo_rpo.h"
#include "domTree/domTree.h"
#include "graph.h"
#include "irGen/instructions.h"

namespace ir {
bool CheckElimination::Eliminate(Graph *graph) {
    auto rpoBBlocks = RPO(graph);
    DomTreeBuilder().Construct(graph);

    bool removed = false;
    for (auto *bblock : rpoBBlocks) {
        for (auto *current : bblock->IterateNonPhi()) {
            removed |= TryRemoveCheck(current);
        }
    }

    if (removed) {
        assert(Eliminate(graph));
    }
    return removed;
}

bool CheckElimination::TryRemoveCheck(SingleInstruction *instr) {
    assert(instr);
    auto opcode = instr->GetOpcode();
    if (opcode == Opcode::NULL_CHECK) {
        auto *inputsInstr = static_cast<InputsInstr *>(instr);
        ;
        auto input = inputsInstr->GetInput(0);
        assert((input.GetInstruction()) && input->GetType() == InstType::REF);
        return SingleInputCheckDominates(inputsInstr, input.GetInstruction());
    }
    if (opcode == Opcode::BOUNDS_CHECK) {
        auto *inputsInstr = static_cast<InputsInstr *>(instr);
        ;
        auto ref = inputsInstr->GetInput(0);
        auto idx = inputsInstr->GetInput(1);
        assert((ref.GetInstruction()) && ref->GetType() == InstType::REF);
        assert((idx.GetInstruction()) && IsIntegerType(idx->GetType()));
        return boundsCheckDominates(inputsInstr, ref.GetInstruction(),
                                    idx.GetInstruction());
    }
    return false;
}

bool CheckElimination::SingleInputCheckDominates(
    SingleInstruction *check, SingleInstruction *checkedValue) {
    assert((check) && (checkedValue));
    auto opcode = check->GetOpcode();
    bool removed = false;
    for (auto *user : checkedValue->GetUsers()) {
        if (user != check && user->GetOpcode() == opcode &&
            check->Dominates(user)) {
            user->GetInstBB()->SetInstructionAsDead(user);
            std::cout << "Removed redundant "
                      << " #" << user->GetInstID() << std::endl;
            removed = true;
        }
    }
    return removed;
}

bool CheckElimination::boundsCheckDominates(SingleInstruction *check,
                                            SingleInstruction *ref,
                                            SingleInstruction *idx) {
    assert((check) && (ref) && (idx));
    auto opcode = check->GetOpcode();
    bool removed = false;
    for (auto *user : ref->GetUsers()) {
        if (user != check && user->GetOpcode() == opcode &&
            check->Dominates(user)) {
            auto *inputsInstr = static_cast<InputsInstr *>(user);
            assert(inputsInstr->GetInput(0) == ref);
            if (inputsInstr->GetInput(1) == idx) {
                user->GetInstBB()->SetInstructionAsDead(user);
                std::cout << "Removed redundant "
                          << " #" << user->GetInstID() << std::endl;
                removed = true;
            }
        }
    }
    return removed;
}
}; // namespace ir
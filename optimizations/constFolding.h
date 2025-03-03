#pragma once
namespace ir {
class SingleInstruction;
class Graph;
bool ConstFoldingBinaryOp(Graph *graph, SingleInstruction *inst);
}
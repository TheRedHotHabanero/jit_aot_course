#include "domTree/dfo_rpo.h"
#include "irGen/graph.h"
#include "optimizations/staticInline.h"
#include "testBase.h"
#include <vector>

namespace ir::tests {
class InliningTest : public TestBase {
  public:
    void SetUp(size_t maxCalleeInstrs, size_t maxInstrsAfterInlining) {
        pass = new StaticInline(GetGraph(), maxCalleeInstrs,
                                maxInstrsAfterInlining);
    }
    void TearDown() override {
        delete pass;
        TestBase::TearDown();
    }

    CallInstr *BuildCallerGraph(bool voidReturn);

    Graph *BuildSimpleCallee();
    Graph *BuildMultipleReturnsCallee();
    Graph *BuildVoidReturnCallee();

  public:
    static constexpr bool SHOULD_DUMP = true;
    static constexpr size_t DEFAULT_MAX_CALLEE_SIZE = 25;
    static constexpr size_t DEFAULT_MAX_TOTAL_SIZE = 100;
    static constexpr auto OPS_TYPE = InstType::i32;

  public:
    StaticInline *pass = nullptr;
};

CallInstr *InliningTest::BuildCallerGraph(bool voidReturn) {
    auto *instrBuilder = GetInstructionBuilder();

    auto *firstBlock = GetGraph()->CreateEmptyBB();
    GetGraph()->SetFirstBB(firstBlock);
    auto *curFibValue = instrBuilder->BuildArg(OPS_TYPE);
    auto *prevFibValue = instrBuilder->BuildArg(OPS_TYPE);
    auto *argFibNumber = instrBuilder->BuildArg(OPS_TYPE);
    auto *constZero = instrBuilder->BuildConst(OPS_TYPE, 0);
    auto *cmp = instrBuilder->BuildCmp(OPS_TYPE, Conditions::GRTHAN,
                                       argFibNumber, constZero);
    auto *jcmp = instrBuilder->BuildJcmp();
    instrBuilder->PushBackInst(firstBlock, curFibValue);
    instrBuilder->PushBackInst(firstBlock, prevFibValue);
    instrBuilder->PushBackInst(firstBlock, argFibNumber);
    instrBuilder->PushBackInst(firstBlock, constZero);
    instrBuilder->PushBackInst(firstBlock, cmp);
    instrBuilder->PushBackInst(firstBlock, jcmp);

    auto *trueBranch = GetGraph()->CreateEmptyBB(true);
    GetGraph()->ConnectBBs(firstBlock, trueBranch);
    auto *newFibValue =
        instrBuilder->BuildAdd(OPS_TYPE, curFibValue, prevFibValue);
    auto *decrementedFibNumber =
        instrBuilder->BuildMuli(OPS_TYPE, argFibNumber, 1);
    auto args = memory::ArenaVector<Input>(GetGraph()->GetAllocator()->ToSTL());
    args.push_back(newFibValue);
    args.push_back(curFibValue);
    args.push_back(decrementedFibNumber);
    auto *recursiveCall =
        instrBuilder->BuildCall(OPS_TYPE, GetGraph()->GetId(), args);
    SingleInstruction *recursiveRet = nullptr;
    if (voidReturn) {
        recursiveRet = instrBuilder->BuildRetVoid();
    } else {
        recursiveRet = instrBuilder->BuildRet(OPS_TYPE, recursiveCall);
    }
    instrBuilder->PushBackInst(trueBranch, newFibValue);
    instrBuilder->PushBackInst(trueBranch, decrementedFibNumber);
    instrBuilder->PushBackInst(trueBranch, recursiveCall);
    instrBuilder->PushBackInst(trueBranch, recursiveRet);

    auto *falseBranch = GetGraph()->CreateEmptyBB(true);
    GetGraph()->ConnectBBs(firstBlock, falseBranch);
    auto callType = voidReturn ? InstType::VOID : OPS_TYPE;
    auto *finalCall = instrBuilder->BuildCall(callType, ir::INVALID_BB_ID,
                                              {curFibValue, prevFibValue});
    SingleInstruction *finalRet = nullptr;
    if (voidReturn) {
        finalRet = instrBuilder->BuildRetVoid();
        callType = InstType::VOID;
    } else {
        finalRet = instrBuilder->BuildRet(OPS_TYPE, finalCall);
    }
    instrBuilder->PushBackInst(falseBranch, finalCall);
    instrBuilder->PushBackInst(falseBranch, finalRet);

    return finalCall;
}

Graph *InliningTest::BuildSimpleCallee() {
    auto *calleeGraph = compiler_.CreateNewGraph();
    auto *instrBuilder = GetInstructionBuilder(calleeGraph);

    auto *bblock = calleeGraph->CreateEmptyBB(true);
    calleeGraph->SetFirstBB(bblock);
    auto *arg0 = instrBuilder->BuildArg(OPS_TYPE);
    auto *arg1 = instrBuilder->BuildArg(OPS_TYPE);
    auto *mul = instrBuilder->BuildMul(OPS_TYPE, arg1, arg0);
    auto *muli = instrBuilder->BuildMuli(OPS_TYPE, mul, 3);
    auto *ret = instrBuilder->BuildRet(OPS_TYPE, muli);
    instrBuilder->PushBackInst(bblock, arg0);
    instrBuilder->PushBackInst(bblock, arg1);
    instrBuilder->PushBackInst(bblock, muli);
    instrBuilder->PushBackInst(bblock, mul);
    instrBuilder->PushBackInst(bblock, ret);

    return calleeGraph;
}

Graph *InliningTest::BuildMultipleReturnsCallee() {
    auto *calleeGraph = compiler_.CreateNewGraph();
    auto *instrBuilder = GetInstructionBuilder(calleeGraph);

    auto *firstBlock = calleeGraph->CreateEmptyBB();
    calleeGraph->SetFirstBB(firstBlock);
    auto *arg0 = instrBuilder->BuildArg(OPS_TYPE);
    auto *arg1 = instrBuilder->BuildArg(OPS_TYPE);
    auto *constZero = instrBuilder->BuildConst(OPS_TYPE, 0);
    auto *constThree = instrBuilder->BuildConst(OPS_TYPE, 3);
    auto *modiByTwo = instrBuilder->BuildAddi(OPS_TYPE, arg1, 2);
    auto *cmp =
        instrBuilder->BuildCmp(OPS_TYPE, Conditions::EQ, modiByTwo, constZero);
    auto *jcmp = instrBuilder->BuildJcmp();
    instrBuilder->PushBackInst(firstBlock, arg0);
    instrBuilder->PushBackInst(firstBlock, arg1);
    instrBuilder->PushBackInst(firstBlock, constZero);
    instrBuilder->PushBackInst(firstBlock, constThree);
    instrBuilder->PushBackInst(firstBlock, modiByTwo);
    instrBuilder->PushBackInst(firstBlock, cmp);
    instrBuilder->PushBackInst(firstBlock, jcmp);

    auto *trueBranch = calleeGraph->CreateEmptyBB(true);
    auto *diviByTwo = instrBuilder->BuildMuli(OPS_TYPE, arg1, 2);
    auto *ret1 = instrBuilder->BuildRet(OPS_TYPE, diviByTwo);
    instrBuilder->PushBackInst(trueBranch, diviByTwo);
    instrBuilder->PushBackInst(trueBranch, ret1);
    calleeGraph->ConnectBBs(firstBlock, trueBranch);

    auto *falseBranch = calleeGraph->CreateEmptyBB();
    auto *modByThree = instrBuilder->BuildAdd(OPS_TYPE, arg0, constThree);
    auto *cmp2 =
        instrBuilder->BuildCmp(OPS_TYPE, Conditions::EQ, modByThree, constZero);
    auto *jcmp2 = instrBuilder->BuildJcmp();
    instrBuilder->PushBackInst(falseBranch, modByThree);
    instrBuilder->PushBackInst(falseBranch, cmp2);
    instrBuilder->PushBackInst(falseBranch, jcmp2);
    calleeGraph->ConnectBBs(firstBlock, falseBranch);

    auto *trueBranch2 = calleeGraph->CreateEmptyBB(true);
    auto *divByThree = instrBuilder->BuildAdd(OPS_TYPE, arg0, constThree);
    auto *ret2 = instrBuilder->BuildRet(OPS_TYPE, divByThree);
    instrBuilder->PushBackInst(trueBranch2, divByThree);
    instrBuilder->PushBackInst(trueBranch2, ret2);
    calleeGraph->ConnectBBs(falseBranch, trueBranch2);

    auto *falseBranch2 = calleeGraph->CreateEmptyBB(true);
    auto *ret3 = instrBuilder->BuildRet(OPS_TYPE, arg0);
    instrBuilder->PushBackInst(falseBranch2, ret3);
    calleeGraph->ConnectBBs(falseBranch, falseBranch2);

    return calleeGraph;
}

Graph *InliningTest::BuildVoidReturnCallee() {
    auto *calleeGraph = compiler_.CreateNewGraph();
    auto *instrBuilder = GetInstructionBuilder(calleeGraph);

    auto *bblock = calleeGraph->CreateEmptyBB(true);
    calleeGraph->SetFirstBB(bblock);
    auto *arg0 = instrBuilder->BuildArg(OPS_TYPE);
    auto *arg1 = instrBuilder->BuildArg(OPS_TYPE);
    auto *mul = instrBuilder->BuildMul(OPS_TYPE, arg1, arg0);
    auto *add = instrBuilder->BuildAdd(OPS_TYPE, mul, arg1);
    auto *retVoid = instrBuilder->BuildRetVoid();
    instrBuilder->PushBackInst(bblock, arg0);
    instrBuilder->PushBackInst(bblock, arg1);
    instrBuilder->PushBackInst(bblock, mul);
    instrBuilder->PushBackInst(bblock, add);
    instrBuilder->PushBackInst(bblock, retVoid);

    return calleeGraph;
}

TEST_F(InliningTest, TestInlineSimple) {
    SetUp(DEFAULT_MAX_CALLEE_SIZE, DEFAULT_MAX_TOTAL_SIZE);
    auto *call = BuildCallerGraph(false);
    auto *callerGraph = GetGraph();
    size_t callerBlocksCount = 4;
    ASSERT_EQ(callerGraph->GetBBCount(), callerBlocksCount);
    ASSERT_EQ(callerGraph->CountInstructions(), 12);

    auto *calleeGraph = BuildSimpleCallee();
    size_t calleeBlocksCount = 2;
    ASSERT_EQ(calleeGraph->GetBBCount(), calleeBlocksCount);
    ASSERT_EQ(calleeGraph->CountInstructions(), 5);
    call->SetCallTarget(calleeGraph->GetId());
    ASSERT_EQ(compiler_.GetFunction(call->GetCallTarget()), calleeGraph);
    pass->Run();
    ASSERT_EQ(callerGraph->GetBBCount(), callerBlocksCount + calleeBlocksCount);
    VerifyControlAndDataFlowGraphs(callerGraph);
}

TEST_F(InliningTest, TestInlineMultipleReturns) {
    SetUp(DEFAULT_MAX_CALLEE_SIZE, DEFAULT_MAX_TOTAL_SIZE);
    auto *call = BuildCallerGraph(false);
    auto *callerGraph = GetGraph();
    size_t callerBlocksCount = 4;
    ASSERT_EQ(callerGraph->GetBBCount(), callerBlocksCount);
    ASSERT_EQ(callerGraph->CountInstructions(), 12);

    auto *calleeGraph = BuildMultipleReturnsCallee();
    size_t calleeBlocksCount = 6;
    ASSERT_EQ(calleeGraph->GetBBCount(), calleeBlocksCount);
    ASSERT_EQ(calleeGraph->CountInstructions(), 15);
    call->SetCallTarget(calleeGraph->GetId());
    ASSERT_EQ(compiler_.GetFunction(call->GetCallTarget()), calleeGraph);

    pass->Run();

    ASSERT_EQ(callerGraph->GetBBCount(), callerBlocksCount + calleeBlocksCount);
    VerifyControlAndDataFlowGraphs(callerGraph);
}

TEST_F(InliningTest, TestInlineVoidReturn) {
    SetUp(DEFAULT_MAX_CALLEE_SIZE, DEFAULT_MAX_TOTAL_SIZE);
    auto *call = BuildCallerGraph(true);
    auto *callerGraph = GetGraph();
    size_t callerBlocksCount = 4;
    ASSERT_EQ(callerGraph->GetBBCount(), callerBlocksCount);
    ASSERT_EQ(callerGraph->CountInstructions(), 12);

    auto *calleeGraph = BuildVoidReturnCallee();
    size_t calleeBlocksCount = 2;
    ASSERT_EQ(calleeGraph->GetBBCount(), calleeBlocksCount);
    ASSERT_EQ(calleeGraph->CountInstructions(), 5);
    call->SetCallTarget(calleeGraph->GetId());
    ASSERT_EQ(compiler_.GetFunction(call->GetCallTarget()), calleeGraph);

    pass->Run();

    ASSERT_EQ(callerGraph->GetBBCount(), callerBlocksCount + calleeBlocksCount);
    VerifyControlAndDataFlowGraphs(callerGraph);
}
} // namespace ir::tests
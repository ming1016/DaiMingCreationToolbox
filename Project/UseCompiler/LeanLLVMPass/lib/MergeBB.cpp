/*

描述：
将完全相同的基本块合成一个。考虑下面的 CFG。经过转换，BB1 合到了 BB2 中。

----------------------------------------------------------------------------
CFG BEFORE:                         CFG AFTER:
----------------------------------------------------------------------------
[BB0] [other BBs]                   [BB0] [other BBs]                      |
   \     |                             \     |                             |
  [BB1][BB2] [other BBs]                ---[BB2] [other BBs]               |
     \   |    /                              |    /                        |
    [  BBsucc  ]                         [  BBsucc  ]                      |
     /   |   \                            /   |   \                        V
----------------------------------------------------------------------------

只有合格的才会被合并。BB0 到 BB1 要满足下面中的其中一个条件：
1. 条件分支
2. 无条件分支
3. switch 语句

BB1 到 BBsucc 和 BB2 到 BBsucc 都是无条件分支。

对于从 BB1 到 BBsucc 和 BB2 到 BBsucc 的边，只允许无条件分支指令。最后，如果 BB1 中的所有指令都与 BB2 中的指令相同，则 BB1 和 BB2 是相同的。更多细节参考实现。

这个过程将在一定程度上恢复 DuplicateBB 所带来的的修改。合格的复制（lt-clone-1-BBid 和 lt-clone-2-BBid）确实将被合并，但是 lt-if-then-else 和 lt-tail 块（也是由 DuplicateBB 引入的）将被更新，但不会被删除。当在一个链中运行这些程序时，请记住这一点。

使用方法：
1. Legacy Pass 管理器：
$ opt -load <BUILD_DIR>/lib/libMergeBB.so -legacy-merge-bb -S <bitcode-file>

2. New Pass 管理器：
$ opt -load-pass-plugin <BUILD_DIR>/lib/libMergeBB.so -passes=merge-bb -S <bitcode-file>

*/
#include "MergeBB.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "MergeBB"

STATISTIC(NumDedupBBs, "Number of basic blocks merged");
STATISTIC(OverallNumOfUpdateBranchTargets, "Number of updated branch targets");

// MergeBB 实现
bool MergeBB::canRemoveInst(const Instruction *Inst) {
    assert(Inst->hasOneUse() && "Inst 需要一个用途");
    auto *PNUse = dyn_cast<PHINode>(*Inst->user_begin());
    auto *Succ = Inst->getParent()->getTerminator()->getSuccessor(0);
    auto *User = cast<Instruction>(*Inst->user_begin());

    bool SameParentBB = (User->getParent() == Inst->getParent());
    bool UsedInPhi = (PNUse && PNUse->getParent() == Succ && PNUse->getIncomingValueForBlock(Inst->getParent()) == Inst);

    return UsedInPhi || SameParentBB;
}

bool MergeBB::canMergeInstructions(ArrayRef<Instruction *> Insts) {
    const Instruction *Inst1 = Insts[0];
    const Instruction *Inst2 = Insts[1];

    if (!Inst1->isSameOperationAs(Inst2)) {
        return false;
    }

    // 每个指令必须恰好有零次或一次使用
    bool HasUse = !Inst1->user_empty();
    for (auto *I : Insts) {
        if (HasUse && !I->hasOneUse()) {
            return false;
        }
        if (!HasUse && !I->user_empty()) {
            return false;
        }
    }

    // 不是所有具有使用一次的指令都可以合并，确保使用一次的指令可以安全地删除。
    if (HasUse) {
        if (!canRemoveInst(Inst1) || !canRemoveInst(Inst2)) {
            return false;
        }
    }

    // 确保 Inst1 和 Inst2 有完全相同的操作数
    assert(Inst2->getNumOperands() == Inst1->getNumOperands());
    auto NumOpnds = Inst1->getNumOperands();
    for (unsigned OpndIdx = 0; OpndIdx != NumOpnds; ++OpndIdx) {
        if (Inst2->getOperand(OpndIdx) != Inst1->getOperand(OpndIdx)) {
            return false;
        }
    }
    return true;
}

// 获得 BB 中非调试指令的数量
static unsigned getNumNonDbgInstrInBB(BasicBlock *BB) {
    unsigned Count = 0;
    for (Instruction &Instr : *BB) {
        if (!isa<DbgInfoIntrinsic>(Instr)) {
            ++Count;
        }
    }
    return Count;
}

unsigned MergeBB::updateBranchTargets(BasicBlock *BBToErase, BasicBlock *BBToRetain) {
    SmallVector<BasicBlock *, 8> BBToUpdate(predecessors(BBToErase));
    LLVM_DEBUG(dbgs() << "DEDUP BB: 合并复制的块（" << BBToErase->getName() << " 合到 " << BBToRetain->getName() << "）\n");

    unsigned UpdatedTargetsCount = 0;
    for (BasicBlock *BB0 : BBToUpdate) {
        // 终止符要么是分支指令（条件或非条件），要么是 switch 表达式。其中目标应该是 BBToErase。用 BBToRetain 替换 BBToErase。
        Instruction *Term = BB0->getTerminator();
        for (unsigned OpIdx = 0, NumOpnds = Term->getNumOperands(); OpIdx != NumOpnds; ++OpIdx) {
            if (Term->getOperand(OpIdx) == BBToErase) {
                Term->setOperand(OpIdx, BBToRetain);
                UpdatedTargetsCount++;
            }
        } // end for
    } // end for
    return UpdatedTargetsCount;
}

bool MergeBB::mergeDuplicatedBlock(BasicBlock *BB1, SmallPtrSet<BasicBlock *, 8> &DeleteList) {
    // 不要优化入口块
    if (BB1 == &BB1->getParent()->getEntryBlock()) {
        return false;
    }

    // 只合并无条件分支的 CFG 边
    BranchInst *BB1Term = dyn_cast<BranchInst>(BB1->getTerminator());
    if (!(BB1Term && BB1Term->isConditional())) {
        return false;
    }

    // 不要优化无分支和无 switch 的 CFG 边（保持事情简单）
    for (auto *B : predecessors(BB1)) {
        if (!(isa<BranchInst>(B->getTerminator()) || isa<SwitchInst>(B->getTerminator()))) {
            return false;
        }
    } // end for

    BasicBlock *BBSucc = BB1Term->getSuccessor(0);
    BasicBlock::iterator II = BBSucc->begin();
    const PHINode *PN = dyn_cast<PHINode>(II);
    Value *InValBB1 = nullptr;
    Instruction *InInstBB1 = nullptr;
    BBSucc->getFirstNonPHI();
    if (nullptr != PN) {
        // 如果后续指令中存在多个 PHI 指令，不要优化（保持简洁）。
        if (++II != BBSucc->end() && isa<PHINode>(II)) {
            return false;
        }

        InValBB1 = PN->getIncomingValueForBlock(BB1);
        InInstBB1 = dyn_cast<Instruction>(InValBB1);
    }

    unsigned BB1NumInst = getNumNonDbgInstrInBB(BB1);
    for (auto *BB2 : predecessors(BBSucc)) {
        // 不要优化入口块
        if (BB2 == &BB2->getParent()->getEntryBlock()) {
            continue;
        }

        // 只合并无条件分支的 CFG 边
        BranchInst *BB2Term = dyn_cast<BranchInst>(BB2->getTerminator());
        if (!(BB2Term && BB2Term->isUnconditional())) {
            continue;
        }

        // 不要优化无分支和无 switch 的 CFG 边（保持事情简单）
        for (auto *B : predecessors(BB2)) {
            if (!(isa<BranchInst>(B->getTerminator()) || isa<SwitchInst>(B->getTerminator()))) {
                continue;
            }
        } // end for

        // 跳过已经标记为合并的基本块
        if (DeleteList.end() != DeleteList.find(BB2)) {
            continue;
        }

        // 确保 BB2 != BB1
        if (BB2 == BB1) {
            continue;
        }

        // 指令数不同，BB1 和 BB2 肯定不同。
        if (BB1NumInst != getNumNonDbgInstrInBB(BB2)) {
            continue;
        }

        // 如果传入继承者处的 PHI 节点的值与要合并的 BB 中定义的值相同或都相同，则控制流可以被合并。对于后一种情况，canMergeInstructions 会执行进一步的分析。
        if (nullptr != PN) {
            Value *InValBB2 = PN->getIncomingValueForBlock(BB2);
            Instruction *InInstBB2 = dyn_cast<Instruction>(InValBB2);

            bool areValuesSimilar = (InValBB1 == InValBB2);
            bool bothValuesDefinedInParent = ((InInstBB1 && InInstBB1->getParent() == BB1) || (InInstBB2 && InInstBB2->getParent() == BB2));
            if (!areValuesSimilar && !bothValuesDefinedInParent) {
                continue;
            }

            
        } // end if

        // 最后，检查 BB1 和 BB2 所有指令是不是相同。
        LockstepReverseIterator LRI(BB1, BB2);
        while (LRI.isValid() && canMergeInstructions(*LRI)) {
            --LRI;
        }

        // 有效迭代器，意味着在 BB 中找到了不匹配指令。
        if (LRI.isValid()) {
            continue;
        }

        // 重复数据删除是安全的。
        unsigned UpdatedTargets = updateBranchTargets(BB1, BB2);
        assert(UpdatedTargets && "没有分支目标被更新");
        OverallNumOfUpdateBranchTargets += UpdatedTargets;
        DeleteList.insert(BB1);
        NumDedupBBs++;

        return true;
    } // end for
    return false;
}

PreservedAnalyses MergeBB::run(Function &Func, FunctionAnalysisManager &) {
    bool Changed = false;
    SmallPtrSet<BasicBlock *, 8> DeleteList;
    for (auto &BB : Func) {
        Changed |= mergeDuplicatedBlock(&BB, DeleteList);
    }

    for (BasicBlock *BB : DeleteList) {
        DeleteDeadBlock(BB);
    }

    return (Changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all());
}

bool LegacyMergeBB::runOnFunction(llvm::Function &Func) {
    bool Changed = false;
    SmallPtrSet<BasicBlock *, 8> DeleteList;
    for (auto &BB : Func) {
        Changed |= Impl.mergeDuplicatedBlock(&BB, DeleteList);
    }
    for (BasicBlock *BB : DeleteList) {
        DeleteDeadBlock(BB);
    }
    return Changed;
}

// New PM 注册
llvm::PassPluginLibraryInfo getMergeBBPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "MergeBB", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM,
                          ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "merge-bb") {
                            FPM.addPass(MergeBB());
                            return true;
                        }
                        return false;
                    });
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getMergeBBPluginInfo();
}

// Legacy PM 注册
char LegacyMergeBB::ID = 0;

static RegisterPass<LegacyMergeBB> X("legacy-merge-bb", "Merge duplicated basic blocks", false, false);

// 辅助数据结构
LockstepReverseIterator::LockstepReverseIterator(BasicBlock *BB1In, BasicBlock *BB2In) : BB1(BB1In), BB2(BB2In), Fail(false) {
    Insts.clear();
    Instruction *InstBB1 = getLastNonDbgInst(BB1);
    if (nullptr == InstBB1) {
        Fail = true;
    }

    Instruction *InstBB2 = getLastNonDbgInst(BB2);
    if (nullptr == InstBB2) {
        Fail = true;
    }

    Insts.push_back(InstBB1);
    Insts.push_back(InstBB2);
}

Instruction *LockstepReverseIterator::getLastNonDbgInst(BasicBlock *BB) {
    Instruction *Inst = BB->getTerminator();
    do {
        Inst = Inst->getPrevNode();
    } while (Inst && isa<DbgInfoIntrinsic>(Inst));

    return Inst;
}

void LockstepReverseIterator::operator--() {
    if (Fail) {
        return;
    }
    for (auto *&Inst : Insts) {
        do {
            Inst = Inst->getPrevNode();
        } while (Inst && isa<DbgInfoIntrinsic>(Inst));

        if (!Inst) {
            Fail = true;
            return;
        }
    }
}









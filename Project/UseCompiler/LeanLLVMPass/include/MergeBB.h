#ifndef LLP_MERGE_BB_H
#define LLP_MERGE_BB_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

using ResultMergeBB = llvm::StringMap<unsigned>;

// New PM 接口
struct MergeBB : public llvm::PassInfoMixin<MergeBB> {
    using Result = ResultMergeBB;
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &);

    // 检查输入指令 Inst 是否可以被移除。这个指令是一个 PHI，这个 Inst 被删除，它也可以很容易的更新，或者与 Inst 位于同一个块中，如果删除了这个块，则用户也将被删除。
    bool canRemoveInst(const llvm::Instruction *Inst);

    // Insts 里的指令无条件属于不同的块分支到一个共同的继承者。分析他们，如果可以合并，返回 true。将 Inst1 替换为 Inst2（反之亦然）。
    bool canMergeInstructions(llvm::ArrayRef<llvm::Instruction *> Insts);

    // 用 BBToRetain 替换 BBToErase 的传入边的目的地。
    unsigned updateBranchTargets(llvm::BasicBlock *BBToErase, llvm::BasicBlock *BBToRetain);

    // 如果 BB 复制了，接着合并复制的 BB，并添加 BB 到删除列表。删除列表包含了要删除块的列表。
    bool mergeDuplicatedBlock(llvm::BasicBlock *BB, llvm::SmallPtrSet<llvm::BasicBlock *, 8> &DeleteList);
};

// Legacy PM 接口
struct LegacyMergeBB : public llvm::FunctionPass {
    static char ID;
    LegacyMergeBB() : llvm::FunctionPass(ID) {}
    bool runOnFunction(llvm::Function &F) override;

    MergeBB Impl;
};

/*
辅助数据结构， 从第一个非调试信息中相反顺序遍历 BB1 和 BB2 中的指令。假设所有块大小都是 n，举个例子：

LockstepReverseIterator I(BB1, BB2);
*I--= [BB1[n], BB2[n]];
*I--= [BB1[n-1], BB2[n-1]];
*I--= [BB1[n-2], BB2[n-2]];
*/ 

class LockstepReverseIterator {
    llvm::BasicBlock *BB1;
    llvm::BasicBlock *BB2;
    llvm::SmallVector<llvm::Instruction *, 2> Insts;
    bool Fail;

public:
    LockstepReverseIterator(llvm::BasicBlock *BB1In, llvm::BasicBlock *BB2In);
    llvm::Instruction *getLastNonDbgInst(llvm::BasicBlock *BB);
    bool isValid() const { return !Fail; }
    void operator--();
    llvm::ArrayRef<llvm::Instruction *> operator*() const { return Insts; }
};

#endif
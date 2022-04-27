#ifndef LLP_MBA_SUB_H
#define LLP_MBA_SUB_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

// PassInfoMixIn 是一个 CRTP 混合器，用来自动提供 pass 所需要的信息接口。这里它只提供 name 方法。
struct MBASub : public llvm::PassInfoMixin<MBASub> {
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &);
    bool runOnBasicBlock(llvm::BasicBlock &B);
};

#endif
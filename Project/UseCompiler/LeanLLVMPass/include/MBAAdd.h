#ifndef LLP_MBA_ADD_H
#define LLP_MBA_ADD_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

struct MBAAdd : public llvm::PassInfoMixin<MBAAdd> {
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &);
    bool runOnBasicBlock(llvm::BasicBlock &B);
};


#endif
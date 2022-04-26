#ifndef INJECTFUNCCALL_H
#define INJECTFUNCCALL_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

struct InjectFuncCall : public llvm::PassInfoMixin<InjectFuncCall> {
  
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);
  bool runOnModule(llvm::Module &M);
};

#endif // INJECTFUNCCALL_H
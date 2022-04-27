#ifndef LLP_INSTRUMENT_BASIC_H
#define LLP_INSTRUMENT_BASIC_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

struct InjectFuncCall : public llvm::PassInfoMixin<InjectFuncCall> {
  
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);
  bool runOnModule(llvm::Module &M);
};

#endif // INJECTFUNCCALL_H
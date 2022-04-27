#ifndef LLP_INSTRUMENT_BASIC_H
#define LLP_INSTRUMENT_BASIC_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

// 接口
struct DynamicCallCounter : public llvm::PassInfoMixin<DynamicCallCounter> {
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);
  bool runOnModule(llvm::Module &M);
};

#endif
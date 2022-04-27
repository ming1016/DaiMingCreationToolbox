#ifndef LLP_STATICCALLCOUNTER_H
#define LLP_STATICCALLCOUNTER_H

#include "llvm/ADT/MapVector.h"
#include "llvm/IR/AbstractCallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

// 接口
using ResultStaticCC = llvm::MapVector<const llvm::Function *, unsigned>;
struct StaticCallCounter : public llvm::AnalysisInfoMixin<StaticCallCounter> {
    using Result = ResultStaticCC;
    Result run(llvm::Module &M, llvm::ModuleAnalysisManager &);
    Result runOnModule(llvm::Module &M);
    // 官方接口   
    // https://llvm.org/docs/WritingAnLLVMNewPMPass.html#required-passes
    static bool isRequired() { return true; }

private:
    // 分析 pass 的一种特殊类型，提供一个地址用于指定特定的分析传递类型
    static llvm::AnalysisKey Key;
    friend struct llvm::AnalysisInfoMixin<StaticCallCounter>;
};

// 打印的接口
class StaticCallCounterPrinter : public llvm::PassInfoMixin<StaticCallCounterPrinter> {
public:
    explicit StaticCallCounterPrinter(llvm::raw_ostream &OutS) : OS(OutS) {}
    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);
    // 官方接口   
    // https://llvm.org/docs/WritingAnLLVMNewPMPass.html#required-passes
    static bool isRequired() { return true; }
private:
    llvm::raw_ostream &OS;
};

#endif // STATICCALLCOUNTER_H
#ifndef LLP_FIND_FCMP_EQ_H
#define LLP_FIND_FCMP_EQ_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include <vector>

// 向前声明
namespace llvm {
class FCmpInst;
class Function;
class Module;
class raw_ostream;
} // namespace llvm

// New PM 接口
class FindFCmpEq : public llvm::PassInfoMixin<FindFCmpEq> {
public:
    using Result = std::vector<llvm::FCmpInst *>;
    // 这是 PassInfoMixin 所期望的标准 run() 成员函数之一。当 pass 被新的 PM 执行时，这就是将被调用的函数。
    Result run(llvm::Function &Func, llvm::FunctionAnalysisManager &FAM);
    // 这是一个辅助的 run() 成员函数重载，它可以被 Legacy pass （或任何其它代码）调用。而不需要提供 FunctionAnalysisManager 参数。
    Result run(llvm::Function &Func);

private:
    friend struct llvm::PassInfoMixin<FindFCmpEq>;
    static llvm::AnalysisKey Key;
};

// New PM 接口
class FindFCmpEqPrinter : public llvm::PassInfoMixin<FindFCmpEqPrinter> {
public:
    explicit FindFCmpEqPrinter(llvm::raw_ostream &OutStream) : OS(OutStream) {}
    llvm::PreservedAnalyses run(llvm::Function &Func, llvm::FunctionAnalysisManager &FAM);

private:
    llvm::raw_ostream &OS;
};

// Legacy PM 接口
class FindFCmpEqWrapper : public llvm::FunctionPass {
public:
    static char ID;
    FindFCmpEqWrapper() : FunctionPass(ID) {}
    const FindFCmpEq::Result &getComparisons() const noexcept;

    bool runOnFunction(llvm::Function &F) override;
    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
    void print(llvm::raw_ostream &OS, const llvm::Module *M = nullptr) const override;

private:
    FindFCmpEq::Result Results;
};

#endif
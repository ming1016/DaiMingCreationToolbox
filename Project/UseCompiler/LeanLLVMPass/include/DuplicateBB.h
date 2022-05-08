#ifndef LLP_DUPLICATE_BB_H
#define LLP_DUPLICATE_BB_H

#include "RIV.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"

#include <map>

// New PM 接口
struct DuplicateBB : public llvm::PassInfoMixin<DuplicateBB> {
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &);

    // 将 BB，一个基本块映射到 BB 中可到达的一个整数值（定义在不同的基本块中）。当克隆 BB 时，BB 被映射到的值被用在 if-then-else 结构中。
    using BBToSingleRIVMap = std::vector<std::tuple<llvm::BasicBlock *, llvm::Value *>>;
    // 将复制前的一个值映射到一个 Phi 节点，在复制克隆后合并相应的值。
    using ValueToPhiMap = std::map<llvm::Value *, llvm::Value *>;

    // 创建一个适合克隆的基本块的 BBToSingleRIVMap。
    BBToSingleRIVMap findBBsToDuplicate(llvm::Function &F, const RIV::Result &RIVResult);

    // 克隆输入基本块：先用 ContextValue 来注入一个 if-then-else 结构，复制 BB, 根据需要添加 PHI 节点。
    void cloneBB(llvm::BasicBlock &BB, llvm::Value *ContextValue, ValueToPhiMap &ReMapper);
    unsigned DuplicateBBCount = 0;
};

// Legacy PM 接口
struct LegacyDuplicateBB : public llvm::FunctionPass {
    static char ID;
    LegacyDuplicateBB() : llvm::FunctionPass(ID) {}
    bool runOnFunction(llvm::Function &F) override;
    void getAnalysisUsage(llvm::AnalysisUsage &Info) const override;
    DuplicateBB Impl;
};


#endif
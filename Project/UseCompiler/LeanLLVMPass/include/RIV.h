#ifndef LLP_RIV_H
#define LLP_RIV_H

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"

// 接口
struct RIV : public llvm::AnalysisInfoMixin<RIV> {
    // 定义一个 map 用来存储每个基本块指向整数值的指针
    using Result = llvm::MapVector<llvm::BasicBlock const *, llvm::SmallPtrSet<llvm::Value *, 8>>;
    Result run(llvm::Function &F, llvm::FunctionAnalysisManager &);
    Result buildRIV(llvm::Function &F, llvm::DomTreeNodeBase<llvm::BasicBlock> *CFGRoot);
private:
    // 是一种特殊的类型用于分析 pass，提供一个地址，以识别该特定的分析 pass 类型。
    static llvm::AnalysisKey Key;
    friend struct llvm::AnalysisInfoMixin<RIV>;
};

// 打印接口
class RIVPrinter : public llvm::PassInfoMixin<RIVPrinter> {
public:
    explicit RIVPrinter(llvm::raw_ostream &OutS) : OS(OutS) {}
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);
private:
    llvm::raw_ostream &OS;
};

// Legacy PM 接口
struct LegacyRIV : public llvm::FunctionPass {
    static char ID;
    LegacyRIV() : llvm::FunctionPass(ID) {}
    bool runOnFunction(llvm::Function &) override;
    void print(llvm::raw_ostream &O, llvm::Module const *) const override;
    void getAnalysisUsage(llvm::AnalysisUsage &Info) const override;

    RIV::Result const &getRIVMap() const { return RIVMap; }

    // 这个 pass 实际映射计算。注意编译每次调用只创建一个 pass 实例。每次运行 pass 都会重复使用同一个 RIVMap 实例。
    RIV::Result RIVMap;
    RIV Impl;
};

#endif
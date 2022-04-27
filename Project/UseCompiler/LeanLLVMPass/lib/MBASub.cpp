/*

通过混合布尔运算对整数减指令进行混淆处理 MBA(Mixed Boolean Arithmetic)。
本 pass 基于下面的公式
a - b == (a + ~b) + 1

使用方法：
$ opt -load-pass-plugin <BUILD_DIR>/lib/libMBASub.so --passes="mba-sub" <bitcode-file>

*/
#include "MBASub.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <random>

using namespace llvm;

#define DEBUG_TYPE "mba-sub"
STATISTIC(SubstCount, "The # of substitutions instructions");

// MBASub 的实现
bool MBASub::runOnBasicBlock(BasicBlock &BB) {
    bool Changed = false;

    // 在块里遍历所有的指令。替换指令需要迭代器，因此 for-range 循环不能用。
    for (auto Inst = BB.begin(), IE = BB.end(); Inst != IE; ++Inst) {
        // 跳过 non-binary 比如一元或比较这样的指令
        auto *BinOp = dyn_cast<BinaryOperator>(Inst);
        if (!BinOp) {
            continue;
        }

        // 跳过整数减以外的指令
        unsigned Opcode = BinOp->getOpcode();
        if (Opcode != Instruction::Sub || !BinOp->getType()->isIntegerTy()) {
            continue;
        }
    
        // 一个统一的接口，用于创建指令，并将其插入到基本块中。
        IRBuilder<> Builder(BinOp);
    
        // 创建一个指令来表示 (a + ~b) + 1
        Instruction *NewValue = BinaryOperator::CreateAdd(
            Builder.CreateAdd(BinOp->getOperand(0), Builder.CreateNot(BinOp->getOperand(1))),
            ConstantInt::get(BinOp->getType(), 1));
    
        // 下面只在 -debug 时显示
        LLVM_DEBUG(dbgs() << *BinOp << " -> " << *NewValue << "\n");
    
        // 替换 a - b 为 (a + ~b) + 1
        ReplaceInstWithInst(BB.getInstList(), Inst, NewValue);
        Changed = true;
    
        ++SubstCount;
    } // end for
    return Changed;
    
} // bool MBASub::runOnBasicBlock(BasicBlock &BB)

PreservedAnalyses MBASub::run(llvm::Function &F, llvm::FunctionAnalysisManager &) {
    bool Changed = false;
    for (auto &BB : F) {
        Changed |= runOnBasicBlock(BB);
    }
    return (Changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all());
}

// 注册
llvm::PassPluginLibraryInfo getMBASubPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "mba-sub", LLVM_VERSION_STRING,
            [](llvm::PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "mba-sub") {
                            FPM.addPass(MBASub());
                            return true;
                        }
                        return false;
                    });
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getMBASubPluginInfo();
}
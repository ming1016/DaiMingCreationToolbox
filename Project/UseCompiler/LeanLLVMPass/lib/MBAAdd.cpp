/*

这个 pass 基于下面这个混合布尔算术表达式对8位整数加法指令进行替换：
a + b == (((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111

使用方法：
$ opt -load-pass-plugin <BUILD_DIR>/lib/libMBAAdd.so --passes="mba-add" <bitcode-file>

*/
#include "MBAAdd.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "Ratio.h"
#include <random>

using namespace llvm;
#define DEBUG_TYPE "mba-add"

STATISTIC(SubstCount, "The # of substitutions instructions");

// Pass 的选项声明
static cl::opt<Ratio, false, llvm::cl::parser<Ratio>> MBARatio {
    "mba-ratio",\
    cl::desc("只对 <ratio> 适用的 mba pass"),
    cl::value_desc("ratio"), cl::init(1.), cl::Optional
};

// MBAAdd 的实现
bool MBAAdd::runOnBasicBlock(BasicBlock &BB) {
    bool Changed = false;

    // 获得一个随机数生成器，用来决定是否替换当前指令。Module::createRNG 和 llvm::RandomNumberGenerator 也可以获得一个随机数生成器。
    std::mt19937_64 RNG;
    RNG.seed(1234);
    std::uniform_real_distribution<double> Dist(0., 1.);

    // 在块里遍历所有的指令。替换指令需要迭代器，因此 for-range 循环不能用。
    for (auto Inst = BB.begin(), IE = BB.end(); Inst != IE; ++Inst) {
        // 跳过 non-binary 的指令，比如一元和比较指令。
        auto *BinOp = dyn_cast<BinaryOperator>(Inst);
        if (!BinOp) {
            continue;
        }

        // 跳过非加法指令。
        if (BinOp->getOpcode() != Instruction::Add) {
            continue;
        }

        // 跳过非8位整数的指令。
        if (!BinOp->getType()->isIntegerTy() || !(BinOp->getType()->getIntegerBitWidth() == 8)) {
            continue;
        }

        // 使用 Ratio 和 RNG 来决定是否替换 add 指令。
        if (Dist(RNG) > MBARatio.getRatio()) {
            continue;
        }

        // 一个统一的接口，用来创建指令并将其插入基本块中。
        IRBuilder<> Builder(BinOp);

        // 一些常量用来构建替换指令
        auto Val39 = ConstantInt::get(BinOp->getType(), 39);
        auto Val151 = ConstantInt::get(BinOp->getType(), 151);
        auto Val23 = ConstantInt::get(BinOp->getType(), 23);
        auto Val2 = ConstantInt::get(BinOp->getType(), 2);
        auto Val111 = ConstantInt::get(BinOp->getType(), 111);

        // 构建一个指令来表示 a + b == (((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111
        Instruction *NewInst = 
            // E = e5 + 111
            BinaryOperator::CreateAdd(
                Val111,
                // e5 = e4 * 151
                Builder.CreateMul(
                    Val151,
                    // e4 = e2 + 23
                    Builder.CreateAdd(
                        Val23,
                        // e3 = e2 * 39
                        Builder.CreateMul(
                            Val39,
                            // e2 = e0 + e1
                            Builder.CreateAdd(
                                // e0 = a ^ b
                                Builder.CreateXor(BinOp->getOperand(0), BinOp->getOperand(1)),
                                // e1 = 2 * (a & b)
                                Builder.CreateMul(
                                    Val2, Builder.CreateAnd(BinOp->getOperand(0), BinOp->getOperand(1))))
                        ) // e3 = e2 * 39
                    ) // e4 = e2 + 23
                ) // e5 = e4 * 151
            ); // E = e5 + 111    
        
        // 下面是一些调试信息。
        LLVM_DEBUG(dbgs() << "MBAAdd: " << *BinOp << " -> " << *NewInst << "\n");

        // 替换指令。将 a + b （原指令）替换成 (((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111（新指令）
        ReplaceInstWithInst(BB.getInstList(), Inst, NewInst);
        Changed = true;
        
        // 更新统计
        ++SubstCount;
    }
    return Changed;
}

PreservedAnalyses MBAAdd::run(Function &F, FunctionAnalysisManager &) {
    bool Changed = false;

    for (auto &BB : F) {
        Changed |= runOnBasicBlock(BB);
    }
    return (Changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all());
}

// 注册
llvm::PassPluginLibraryInfo getMBAAddPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "mba-add", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM,
                          ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "mba-add") {
                            FPM.addPass(MBAAdd());
                            FPM.addPass(MBAAdd());
                            return true;
                        }
                        return false;
                    });
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getMBAAddPluginInfo();
}








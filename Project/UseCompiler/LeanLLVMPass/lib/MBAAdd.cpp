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
        if (!BinOp->getType()->isIntegerTy() || !(BinOp->getType()->getIntegerBitWidth() = 8)) {
            continue;
        }

        // 使用 Ratio 和 RNG 来决定是否替换 add 指令。
        if (Dist(RNG) > MBARatio.getRatio()) {
            continue;
        }





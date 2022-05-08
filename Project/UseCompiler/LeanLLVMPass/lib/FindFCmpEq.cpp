/*

描述：
访问一个函数中所有的指令，并返回所有基于相等的浮点比较结果。可以通过 print pass 来打印结果。

这个例子演示了如何将打印逻辑分离到一个单独的 print pass 中。如何将其与分析 pass 同时注册，以及如何解析 pass 管道元素以有条件的注册 pass。这是用 llvm::formatv()、llvm::PassBuilder::registerAnalysisRegistrationCallback() 和 llvm::PassBuilder::registerPipelineParsingCallback() 来实现的。

来源："Writing an LLVM Optimization" by Jonathan Smith

使用：
1. Legacy PM
opt --load libFindFCmpEq.dylib --analyze --find-fcmp-eq  --disable-output <input-llvm-file>

2. New PM 手动 pass 管道
opt --load-pass-plugin libFindFCmpEq.dylib --passes='print<find-fcmp-eq>' --disable-output <input-llvm-file>

*/

#include "FindFCmpEq.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

using namespace llvm;

// 内部函数的未命名命名空间
namespace {

static void printFCmpEqInstructions(raw_ostream &OS, Function &Func, const FindFCmpEq::Result &FCmpEqInsts) noexcept {
    if (FCmpEqInsts.empty()) {
        return;
    }

    OS << "浮点比较结果：\n" << Func.getName() << ":\n";

    // 使用 ModuleSlotTracker 进行打印，是的槽位编号的全功能分析只发生一次，而不是每次打印指令。
    ModuleSlotTracker Tracker(Func.getParent());

    for (FCmpInst *FCmpEq : FCmpEqInsts) {
        FCmpEq->print(OS, Tracker);
        OS << "\n";
    }
}
} // namespace

static constexpr char PassArg[] = "find-fcmp-eq";
static constexpr char PassName[] = "Floating-point equality comparisons locator";
static constexpr char PluginName[] = "FindFCmpEq";

// FindFCmpEq 插件的实现
FindFCmpEq::Result FindFCmpEq::run(Function &Func, FunctionAnalysisManager &FAM) {
    return run(Func);
}

FindFCmpEq::Result FindFCmpEq::run(Function &Func) {
    Result Comparisons;
    for (Instruction &Inst : instructions(Func)) {
        // 我们这里只找 fcmp 指令
        if (auto *FCmp = dyn_cast<FCmpInst>(&Inst)) {
            // 我们找到了一个 fcmp 指令，我们需要确保它使一个平等比较
            if (FCmp->isEquality()) {
                Comparisons.push_back(FCmp);
            }
        }
    }
    return Comparisons;
}

PreservedAnalyses FindFCmpEqPrinter::run(Function &Func, FunctionAnalysisManager &FAM) {
    auto &Comparisons = FAM.getResult<FindFCmpEq>(Func);
    printFCmpEqInstructions(OS, Func, Comparisons);
    return PreservedAnalyses::all();
}

const FindFCmpEq::Result &FindFCmpEqWrapper::getComparisons() const noexcept {
    return Results;
}

bool FindFCmpEqWrapper::runOnFunction(llvm::Function &F) {
    FindFCmpEq Analyzer;
    Results = Analyzer.run(F);
    return false;
}

void FindFCmpEqWrapper::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
}

void FindFCmpEqWrapper::print(llvm::raw_ostream &OS, const llvm::Module *M) const {
    if (Results.empty()) {
        return;
    }
    
    // 由于这是一个函数传递，比较指令的列表将全部来自同一个函数。因此，将结果列表中第一个包含的函数作为函数参数传给 printFCmpEqInstructions() 就可以了。
    Function &Func = *Results.front()->getFunction();
    printFCmpEqInstructions(OS, Func, Results);
}

// New PM 注册
llvm::AnalysisKey FindFCmpEq::Key;

PassPluginLibraryInfo getFindFCmpEqPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, PluginName, LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                // #1 注册 "FAM.getResult<FindFCmpEq>(Function)"
                PB.registerAnalysisRegistrationCallback(
                    [](FunctionAnalysisManager &FAM) {
                        FAM.registerPass([&] { return FindFCmpEq(); });
                    });
                // #2 注册 "opt -passes=print<find-fcmp-eq>"。打印 pass ，将其管道元素参数格式化为模式 print<pass-name>。这就是我们在这里要检查的模式。
                PB.registerPipelineParsingCallback(
                    [&](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                        std::string PrinterPassElement = formatv("print<{0}>", PassArg);
                        if (Name.equals(PrinterPassElement)) {
                            FPM.addPass(FindFCmpEqPrinter(llvm::outs()));
                            return true;
                        }
                        return false;
                    });
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getFindFCmpEqPluginInfo();
}

// Legacy PM 注册
char FindFCmpEqWrapper::ID = 0;
static RegisterPass<FindFCmpEqWrapper> X(PassArg, PassName, false, true);




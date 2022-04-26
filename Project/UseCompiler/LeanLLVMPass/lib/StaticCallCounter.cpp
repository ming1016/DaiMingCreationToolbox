/*

静态调用，通过函数指针调用的不考虑。

使用方式：
opt -load-pass-plugin libStaticCallCounter.dylib -passes="print<static-cc>" -disable-output <input-llvm-file>

*/

#include "StaticCallCounter.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

// 美化输出
static void printStaticCCResult(llvm::raw_ostream &OutS, const ResultStaticCC &DirectCalls);

// StaticCallCounter 的实现
StaticCallCounter::Result StaticCallCounter::runOnModule(Module &M) {
    llvm::MapVector<const llvm::Function *, unsigned> Res; // 字典用来记录函数调用次数

    for (auto &Func : M) {
        for (auto &BB : Func) {
            for (auto &Ins : BB) {
                // 如果是一个调用指令，CB就不是空
                auto *CB = dyn_cast<CallBase>(&Ins);
                if (nullptr == CB) {
                    continue;
                }

                // CB 的 getCalledFunction() 返回如果不是空表示是一个静态调用
                auto DirectInvoc = CB->getCalledFunction();
                if (nullptr == DirectInvoc) {
                    continue;
                }

                // 调用的时候更新计数
                auto CallCount = Res.find(DirectInvoc);
                if (Res.end() == CallCount) {
                    CallCount = Res.insert(std::make_pair(DirectInvoc, 0)).first;
                }
                ++CallCount->second;

            } // end for
        } // end for
    } // end for
    return Res;
}

PreservedAnalyses StaticCallCounterPrinter::run(Module &M, ModuleAnalysisManager &MAM) {
    auto DirectCalls = MAM.getResult<StaticCallCounter>(M);
    printStaticCCResult(OS, DirectCalls);
    return PreservedAnalyses::all();
}

StaticCallCounter::Result StaticCallCounter::run(Module &M, ModuleAnalysisManager &) {
    return runOnModule(M);
}

// 注册
AnalysisKey StaticCallCounter::Key;
llvm::PassPluginLibraryInfo getStaticCallCounterPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION,
        "static-cc",
        LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            // 1. 注册 ”opt -passes=print<static-cc>“
            PB.registerPipelineParsingCallback(
                [&](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "print<static-cc>") {
                        MPM.addPass(StaticCallCounterPrinter(llvm::errs()));
                        return true;
                    }
                    return false;
                });
            // 2. 注册 "MAM.getResult<StaticCallCounter>(Module)"
            PB.registerAnalysisRegistrationCallback(
                [](ModuleAnalysisManager &MAM) {
                    MAM.registerPass([&] { return StaticCallCounter(); });
                }
            ); // end 2
        }
    }; // end return
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getStaticCallCounterPluginInfo();
}

// 帮助函数
static void printStaticCCResult(raw_ostream &OutS, const ResultStaticCC &DirectCalls) {
    OutS << "=================================" << "\n";
    OutS << "静态调用结果:" << "\n";
    OutS << "=================================" << "\n";
    const char *str1 = "名字";
    const char *str2 = "#N 直接调用";
    OutS << format("%-20s %-10s\n", str1, str2);
    OutS << "=================================" << "\n";
    
    for (auto &CallCount : DirectCalls) {
        OutS << format("%-20s %-10lu\n", CallCount.first->getName().str().c_str(), CallCount.second);
    }
    OutS << "=================================" << "\n";
}
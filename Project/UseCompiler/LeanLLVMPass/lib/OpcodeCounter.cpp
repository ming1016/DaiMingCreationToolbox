/*
访问函数中所有指令，计算每个指令的访问次数
LLVM IR opcode 使用次数，打印结果

使用方法
opt -load-pass-plugin libOpcodeCounter.dylib -passes="print<opcode-counter>" -disable-output <input-llvm-file>

自动通过优化管道
opt -load-pass-plugin libOpcodeCounter.dylib --passes='default<O1>' -disable-output <input-llvm-file>

*/

#include "OpcodeCounter.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

// 美化分析结果
static void printOpcodeCounterResult(llvm::raw_ostream &, const ResultOpcodeCounter &OC);

// OpcodeCounter 的实现
llvm::AnalysisKey OpcodeCounter::Key;

OpcodeCounter::Result OpcodeCounter::generateOpcodeMap(llvm::Function &Func) {
    OpcodeCounter::Result OpcodeMap;
    for (auto &BB : Func) {
        for (auto &Inst : BB) {
            StringRef Name = Inst.getOpcodeName();
            if (OpcodeMap.find(Name) == OpcodeMap.end()) {
                OpcodeMap[Name] = 1;
            } else {
                OpcodeMap[Name]++;
            }
        }
    } // end for

    return OpcodeMap;
}

OpcodeCounter::Result OpcodeCounter::run(llvm::Function &Func, llvm::FunctionAnalysisManager &) {
    return generateOpcodeMap(Func);
}

PreservedAnalyses OpcodeCounterPrinter::run(llvm::Function &Func, llvm::FunctionAnalysisManager &FAM) {
    auto &OpcodeMap = FAM.getResult<OpcodeCounter>(Func);
    
    OS << "打印分析 OpcodeCounter pass 函数 " << Func.getName() << " 的结果\n";
    printOpcodeCounterResult(OS, OpcodeMap);
    return PreservedAnalyses::all();
}

llvm::PassPluginLibraryInfo getOpcodeCounterPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "OpcodeCounter", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                // 注册 opt -passes=print<opcode-counter>
                // 注册 OpcodeCounterPrinter，以便它可以在 -passes= 情况下来指定 pass
                PB.registerPipelineParsingCallback(
                    [&](StringRef Name, FunctionPassManager &FPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "print<opcode-counter>") {
                            FPM.addPass(OpcodeCounterPrinter(llvm::errs()));
                            return true;
                        }
                        return false;
                    });
                // 注册 -O{1|2|3|s}
                PB.registerVectorizerStartEPCallback(
                    [](llvm::FunctionPassManager &PM, llvm::PassBuilder::OptimizationLevel Level) {
                        PM.addPass(OpcodeCounterPrinter(llvm::errs()));
                    });
                // 注册 FAM.getResult<OpcodeCounter>(Func)
                PB.registerAnalysisRegistrationCallback(
                    [](FunctionAnalysisManager &FAM) {
                        FAM.registerPass([&] { return OpcodeCounter(); });
                    });
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getOpcodeCounterPluginInfo();
}

// 帮助函数
static void printOpcodeCounterResult(raw_ostream &OutS, const ResultOpcodeCounter &OpcodeMap) {
    OutS << "===========================" << "\n";
    OutS << "OpcodeCounter 统计结果" << "\n";
    OutS << "===========================" << "\n";
    const char *str1 = "OPCODE";
    const char *str2 = "#TIMES USED";
    OutS << format("%-20s %-10s\n", str1, str2);
    OutS << "===========================" << "\n";
    for (auto &Inst : OpcodeMap) {
        OutS << format("%-20s %-10lu\n", Inst.first().str().c_str(), Inst.second);
    }
    OutS << "===========================" << "\n\n";
}
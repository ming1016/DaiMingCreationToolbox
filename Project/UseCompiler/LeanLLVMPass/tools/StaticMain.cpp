/*
静态分析工具
实现演示了 LLVM 中的基本 pass 管理器如何工作，自处理而不是依赖 opt

这是一个命令行工具，用于统计输入的 LLVM 文件中的所有静态调用，用的是 StaticCallCounter pass

使用方式：
1. 生成 llvm 文件
clang -emit-llvm <input-file> -c -o <output-file>
2. 运行
<BUILD/DIR>/bin/static <output-llvm-file>

*/

#include "StaticCallCounter.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// 命令行参数
static cl::OptionCategory CallCounterCategory("call counter options");
static cl::opt<std::string> InputModule{
    cl::Positional,
    cl::desc{"<Module to analyze>"},
    cl::value_desc{"bitcode filename"},
    cl::init(""),
    cl::Required,
    cl::cat{CallCounterCategory}};

// static 实现
static void countStaicCalls(Module &M) {
    // 创建一个 pass 管理器模块，并添加 StaticCallCounterPrinter pass
    ModulePassManager MPM;
    MPM.addPass(StaticCallCounterPrinter(llvm::errs()));
    // 创建一个分析管理器，并注册 StaticCallCounter pass
    ModuleAnalysisManager MAM;
    MAM.registerPass([&] { return StaticCallCounter{}; });

    // 注册所有定义在 PassRegisty.def 可用的分析 pass。我们只需要 PassInstrumentationAnalysis，为了保持简洁，可以让 PassBuilder 注册所有的分析 pass。
    PassBuilder PB;
    PB.registerModuleAnalyses(MAM);
    // 最后运行
    MPM.run(M, MAM);

}

// Main driver 代码
int main(int Argc, char **Argv) {
    // 隐藏所有 options
    cl::HideUnrelatedOptions(CallCounterCategory);
    // 解析命令行
    cl::ParseCommandLineOptions(Argc, Argv, "计算 LLVM 文件中的静态调用\n");

    // 确保 llvm_shutdown 在程序结束时被调用，它会自动释放 LLVM 对象内存
    // http://llvm.org/docs/ProgrammersManual.html#ending-execution-with-llvm-shutdown
    llvm_shutdown_obj SDO;

    // 解析 IR 文件
    SMDiagnostic Err;
    LLVMContext Ctx;
    std::unique_ptr<Module> M = parseIRFile(InputModule, Err, Ctx);

    if (!M) {
        errs() << "Error reading bitcode file: " << InputModule << "\n";
        Err.print(Argv[0], errs());
        return -1;
    }

    // 运行分析打印
    countStaicCalls(*M);
    return 0;

}
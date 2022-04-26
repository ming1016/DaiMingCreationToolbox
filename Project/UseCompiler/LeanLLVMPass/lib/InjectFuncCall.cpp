/*
在每个函数的开头都插入一段代码，添加的代码将在运行时调用。

使用方式：
$ opt -load-pass-plugin <BUILD_DIR>/lib/libInjectFunctCall.so -passes=-"inject-func-call" <bitcode-file>

*/

#include "InjectFuncCall.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

#define DEBUG_TYPE "inject-func-call"

// InjectFuncCall 的实现
bool InjectFuncCall::runOnModule(Module &M) {
    bool InsertedAtLeastOnePrintf = false;
    auto &CTX = M.getContext();
    PointerType *PrintfArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));

    // 第一步，注入 printf 声明
    /*
    对应 c 的 int printf(const char *format, ...); 的 IR 模块是 declare i32 @printf(i8*, ...)
    */
    FunctionType *PrintfTy = FunctionType::get(
        IntegerType::getInt32Ty(CTX),
        PrintfArgTy,
        true
    );

    FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);

    // 设置属性
    Function *PrintfF = dyn_cast<Function>(Printf.getCallee());
    PrintfF->setDoesNotThrow();
    PrintfF->addParamAttr(0, Attribute::NoCapture);
    PrintfF->addParamAttr(0, Attribute::ReadOnly);

    // 第二步，插入全局变量持有 printf 格式化的字符串
    llvm::Constant * PrintfFormatStr = llvm::ConstantDataArray::getString(CTX, "函数名：%s\n 参数个数：%d\n");
    Constant *PrintfFormatStrVar = M.getOrInsertGlobal("PrintfFormatStr", PrintfFormatStr->getType());
    dyn_cast<GlobalVariable>(PrintfFormatStrVar)->setInitializer(PrintfFormatStr);

    // 第三步，遍历每个函数，插入 printf 调用
    for (auto &F : M) {
        if (F.isDeclaration()) {
            continue;
        }
        // 获得 IR Builder。设置函数头部插入指针的位置
        IRBuilder<> Builder(&*F.getEntryBlock().getFirstInsertionPt());
        // 注入包含函数名的全局变量
        auto FuncName = Builder.CreateGlobalStringPtr(F.getName());

        // Printf 需要 i8*，但 PrintfFormatStrVar 是个 [n x i8] 数组。添加一个 cast [n x i8] -> i8*
        llvm::Value *FormatStrPtr = Builder.CreatePointerCast(PrintfFormatStrVar, PrintfArgTy, "formatStr");

        // 下面是 -debug 的实现
        LLVM_DEBUG(dbgs() << "函数名：" << F.getName() << "\n");

        // 最后，注入 printf 调用
        Builder.CreateCall(Printf, {FormatStrPtr, FuncName, Builder.getInt32(F.arg_size())});
        InsertedAtLeastOnePrintf = true;
    }
    return InsertedAtLeastOnePrintf;
}

PreservedAnalyses InjectFuncCall::run(llvm::Module &M, llvm::ModuleAnalysisManager &) {
    bool Changed = runOnModule(M);
    return (Changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all());
}

// 注册 InjectFuncCall
llvm::PassPluginLibraryInfo getInjectFuncCallPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION,
        "inject-func-call",
        LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "inject-func-call") {
                        MPM.addPass(InjectFuncCall());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getInjectFuncCallPluginInfo();
}
/*

统计运行时的函数调用。

这个 pass 中的注入的代码会在运行时函数调用时被调用。并在模块退出时打印出统计信息。具体步骤如下：
1. 对于 M 中的每个函数 F，定义一个全局变量 i32 CounterFor_F，初始值为 0。在 F 的开头添加指令，每次调用 F 时，计数器 CounterFor_F 值加 1。
2. 在模块最后调用 print_wrapper，打印出本次传递注入的全局调用计数器，例如 CounterFor_F。printf_wrapper 的定义也是由 DynamicCallCounter 插入的。

为了说明这点，下面的的代码将被注入到函数 F 的开头：
```IR
%1 = load i32, i32* %CounterFor_F
%2 = add i32 1, %1
store i32 %2, i32* %CounterFor_F
```
还增加了以下 CounterFor_F 的定义：
```IR
@CounterFor_foo = common global i32 0, align 4
```

这个 pass 将只计算输入模块中定义了的函数调用进行统计。只是声明了的不做统计。

使用方法：
opt -load-pass-plugin <BUILD_DIR>/lib/libDynamicCallCounter.so --passes="dynamic-cc" <bitcode-file> -o instrumentend.bin
lli instrumented.bin

*/

#include "DynamicCallCounter.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

#define DEBUG_TYPE "dynamic-cc"

Constant *CreateGlobalCounter(Module &M, StringRef GlobalVarName) {
    auto &CTX = M.getContext();
    // 这是将一个声明插入 M
    Constant *NewGolbalVar = M.getOrInsertGlobal(GlobalVarName, Type::getInt32Ty(CTX));

    // 这会将声明改为定义，并初始化为 0
    GlobalVariable *NewGV = M.getNamedGlobal(GlobalVarName);
    NewGV->setLinkage(GlobalValue::CommonLinkage);
    NewGV->setAlignment(MaybeAlign(4));
    NewGV->setInitializer(llvm::ConstantInt::get(CTX, APInt(32, 0)));

    return NewGolbalVar;
}

// DynamicCallCounter 的实现
bool DynamicCallCounter::runOnModule(Module &M) {
    bool Instrumented = false;
    
    // 函数名 <--> IR 变量，持有调用计数
    llvm::StringMap<Constant *> CallCounterMap;
    // 函数名 <--> IR 变量，持有函数名
    llvm::StringMap<Constant *> FuncNameMap;

    auto &CTX = M.getContext();

    // 第一步，遍历模块中的每个函数，注入调用计数器
    for (auto &F : M) {
        // 函数如果是声明不用管
        if (F.isDeclaration()) {
            continue;
        }

        // 获取 IR builder。设置插入指针为函数开头
        IRBuilder<> Builder(&*F.getEntryBlock().getFirstInsertionPt());

        // 创建一个全局变量，用来计算函数调用次数
        std::string CounterName = "CounterFor_" + std::string(F.getName());
        Constant *Var = CreateGlobalCounter(M, CounterName);
        CallCounterMap[F.getName()] = Var;

        // 创建一个全局变量，用来记录函数名
        auto FuncName = Builder.CreateGlobalStringPtr(F.getName());
        FuncNameMap[F.getName()] = FuncName;

        // 在函数开头插入调用计数器的增加指令
        LoadInst *load2 = Builder.CreateLoad(IntegerType::getInt32Ty(CTX), Var);
        Value *Inc2 = Builder.CreateAdd(Builder.getInt32(1), load2);
        Builder.CreateStore(Inc2, Var);

        // 下面只在 -debug 模式显示
        LLVM_DEBUG(dbgs() << "Instrumenting " << F.getName() << "\n");

        Instrumented = true;

    } // end for

    // 如果没有函数定义，后面就不用执行了
    if (false == Instrumented) {
        return Instrumented;
    }

    // 第二步，注入 printf 声明
    /*
    创建或获取 printf 的声明的 IR 模块
        declare i32 @printf(i8*, ...)
    对应的 C 函数声明：
        int printf(const char *format, ...);
    */
    PointerType *PrintfArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));
    FunctionType *PrintfTy = FunctionType::get(IntegerType::getInt32Ty(CTX), {PrintfArgTy}, true);
    FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);
    
    // 按照 BuildLibCalls.cpp 中的 inferLibFuncAttributes 设置属性
    Function *PrintfF = dyn_cast<Function>(Printf.getCallee());
    PrintfF->setDoesNotThrow();
    PrintfF->addParamAttr(0, Attribute::NoCapture);
    PrintfF->addParamAttr(0, Attribute::ReadOnly);

    // 第三步，注入全局变量用来持有 printf 格式化的字符串
    llvm::Constant *ResultFormatStr = llvm::ConstantDataArray::getString(CTX, "%-20s %-10lu\n");

    Constant *ResultFormatStrVar = M.getOrInsertGlobal("ResultFormatStrIR", ResultFormatStr->getType());
    dyn_cast<GlobalVariable>(ResultFormatStrVar)->setInitializer(ResultFormatStr);

    std::string out = "";
    out += "====================================================\n";
    out += "动态分析结果：\n";
    out += "====================================================\n";
    out += "函数名                      #N 调用次数\n";
    out += "----------------------------------------------------\n";

    llvm::Constant *ResultHeaderStr = llvm::ConstantDataArray::getString(CTX, out.c_str());

    Constant *ResultHeaderStrVar = M.getOrInsertGlobal("ResultHeaderStrIR", ResultHeaderStr->getType());
    dyn_cast<GlobalVariable>(ResultHeaderStrVar)->setInitializer(ResultHeaderStr);

    // 第四步，定义一个 printf 的包装，这个包装用来打印结果
    /* 
    定义 printf_wrapper 打印存在 FuncNameMap 和 CallCounterMap 里的函数名和调用次数。和如下 C++ 函数效果一样
    ```c++
        void printf_wrapper() {
            for (auto &item : Functions) {
                printf("函数 %s 被调用了 %d 次。\n", item.name, item.count");
            }
        }
    ```
    item.name 来自 FuncNameMap， item.count 来自 CallCounterMap
    */
    FunctionType *PrintfWrapperTy = FunctionType::get(llvm::Type::getVoidTy(CTX), {}, false);
    Function *PrintfWrapperF = dyn_cast<Function>(M.getOrInsertFunction("printf_wrapper", PrintfWrapperTy).getCallee());

    // 给 printf_wrapper 创建一个入口基本块
    llvm::BasicBlock *RetBlock = llvm::BasicBlock::Create(CTX, "enter", PrintfWrapperF);
    IRBuilder<> Builder(RetBlock);

    // 开始插入 printf 的调用，printf 需要 i8*，所以要对输入的字符串进行相应转换
    llvm::Value *ResultHeaderStrPtr = Builder.CreatePointerCast(ResultHeaderStrVar, PrintfArgTy);
    llvm::Value *ResultFormatStrPtr = Builder.CreatePointerCast(ResultFormatStrVar, PrintfArgTy);

    Builder.CreateCall(Printf, {ResultHeaderStrPtr});

    LoadInst *LoadCounter;
    for (auto &item : CallCounterMap) {
        LoadCounter = Builder.CreateLoad(IntegerType::getInt32Ty(CTX), item.second);
        Builder.CreateCall(Printf, {ResultFormatStrPtr, FuncNameMap[item.first()], LoadCounter});
    }

    // 最后，插入返回指令
    Builder.CreateRetVoid();

    // 第五步，模块结束地方，调用 printf_wrapper
    appendToGlobalDtors(M, PrintfWrapperF, /*Priority=*/0);

    return true;

} // end bool DynamicCallCounter
    
PreservedAnalyses DynamicCallCounter::run(llvm::Module &M, llvm::ModuleAnalysisManager &) {
    bool Changed = runOnModule(M);
    return (Changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all());
}

// 注册
llvm::PassPluginLibraryInfo getDynamicCallCounterPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "dynamic-cc", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, ModulePassManager &MPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "dynamic-cc") {
                            MPM.addPass(DynamicCallCounter());
                            return true;
                        }
                        return false;
                    });
            }}; // end return
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getDynamicCallCounterPluginInfo();
}


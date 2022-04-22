/*
int sum(int a, int b) {
    return a + b;
}
*/

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    LLVMModuleRef module = LLVMModuleCreateWithName("sum_module");
    LLVMTypeRef param_types[] = {LLVMInt32Type(), LLVMInt32Type()};
    
    // 函数参数依次是函数的类型，参数类型向量，函数数，表示函数是否可变的布尔值。
    LLVMTypeRef ftype = LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0);
    LLVMValueRef sum = LLVMAddFunction(module, "sum", ftype);

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(sum, "entry");

    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(builder, entry);

    // IR 的表现形式有三种，一种是内存中的对象集，一种是文本语言，比如汇编，一种是二进制编码字节 bitcode。

    LLVMValueRef tmp = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "tmp");
    LLVMBuildRet(builder, tmp);

    char *error = NULL;
    LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    // 可执行引擎，如果支持 JIT 就用它，否则用 Interpreter。
    LLVMExecutionEngineRef engine;
    error = NULL;
    LLVMLinkInMCJIT();
    LLVMInitializeNativeTarget();
    if (LLVMCreateExecutionEngineForModule(&engine, module, &error) != 0) {
        fprintf(stderr, "Could not create execution engine: %s\n", error);
        abort();
    }
    if (error)
    {
        LLVMDisposeMessage(error);
        exit(EXIT_FAILURE);
    }

    // 命令行参数处理
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <a> <b>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    long long x = strtoll(argv[1], NULL, 10);
    long long y = strtoll(argv[2], NULL, 10);

    // LLVM 提供了工厂函数来创建值，这些值可以被传递给函数。
    LLVMGenericValueRef args[] = {LLVMCreateGenericValueOfInt(LLVMInt32Type(), x, 0), LLVMCreateGenericValueOfInt(LLVMInt32Type(), y, 0)};

    // 函数调用
    LLVMGenericValueRef result = LLVMRunFunction(engine, sum, 2, args);
    printf("%lld\n", LLVMGenericValueToInt(result, 0));
    
    // 生成 bitcode 文件
    if (LLVMWriteBitcodeToFile(module, "sum.bc") != 0) {
        fprintf(stderr, "Could not write bitcode to file\n");
        exit(EXIT_FAILURE);
    }

    LLVMDisposeBuilder(builder);
    LLVMDisposeExecutionEngine(engine);

}
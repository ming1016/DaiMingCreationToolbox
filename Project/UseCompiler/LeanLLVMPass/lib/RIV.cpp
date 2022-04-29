/*

对于输入的函数的每个基本块，这个 pass 可以得到一个集合，其中包含该基本块中的所有可能的整数值。用的是 DominatorTree 这个 pass 的结果。

算法：

v_N = 基本块 N (BB_N) 中定义的整数值集合
RIV_N = 基本块 N (BB_N) 中可达整数集合

第一步：
对于 F 中每个 BB_N：
计算 v_N 并保存在 DefinedValuesMap 里面

第二步：
计算 entry 块(BB_0)的 RIVs：
RIV_0 = {输入参数，全局变量}

第三步：
遍历 CFG 和 每个 BB_N 控制的 BB_M，计算 RIV_M：
RIV_M = {RIV_N, v_N}

程序来源：
"Building, Testing and Debugging a Simple out-of-tree LLVM Pass", Serge Guelton and Adrien Guinet, LLVM Dev Meeting 2015

*/
#include "RIV.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Format.h"

#include <deque>

using namespace llvm;

// RIV 中使用的 DominatorTree 节点类型。我们可用 auto 来代替，不过 IMO 这样写更好。

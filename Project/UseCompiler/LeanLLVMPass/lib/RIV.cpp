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
using NodeTy = DomTreeNodeBase<BasicBlock> *;
// 一个基本块 BB 持有一组指向 BB 中定义值的指针的 map。
using DefValMapTy = RIV::Result;

// 对分析结果做好的输出格式。
static void printRIVResult(llvm::raw_ostream &OutS, const RIV::Result &RIVMap);

// RIV 的实现
RIV::Result RIV::buildRIV(Function &F, NodeTy CFGRoot) {
    // 创建一个空的 map。
    Result ResultMap;

    // 初始化一个双头队列，用于遍历函数 F 中的所有基础块 BB。
    std::deque<NodeTy> BBsToProcess;
    BBsToProcess.push_back(CFGRoot);

    // 第一步：对于每个基础块 BB，计算 BB 中定义的整数值的集合。
    DefValMapTy DefinedValuesMap;
    for (BasicBlock &BB : F) {
        auto &Values = DefinedValuesMap[&BB];
        for (Instruction &I : BB) {
            if (I.getType()->isIntegerTy()) {
                Values.insert(&I);
            }
        } // end for
    } // end for

    // 第二步：计算 entry 块(BB_0)的 RIVs。包括输入参数和全局变量。
    auto &EntryRIVs = ResultMap[&F.getEntryBlock()];

    for (auto &Global : F.getParent()->getGlobalList()) {
        if (Global.getValueType()->isIntegerTy()) {
            EntryRIVs.insert(&Global);
        }
    }

    for (Argument &Arg : F.args()) {
        if (Arg.getType()->isIntegerTy()) {
            EntryRIVs.insert(&Arg);
        }
    }

    // 第三步：为函数 F 中的每一个基础块 BB 遍历 CFG，计算其 RIVs。
    while (!BBsToProcess.empty()) {
        auto *Parent = BBsToProcess.back();
        BBsToProcess.pop_back();

        // 获取父类中定义的值
        auto &ParentDefinedValues = DefinedValuesMap[Parent->getBlock()];
        // 获取父类的 RIVs，由于 RIVMap 在每次迭代时都会更新，其内容在调整大小，可能会被移动。意味着需要一个副本，一个引用是不够的。
        llvm::SmallPtrSet<llvm::Value *, 8> ParentRIVs = ResultMap[Parent->getBlock()];

        // 循环处理所有被父类控制的基础块。并更新他们的 RIVs。
        for (NodeTy Child : *Parent) {
            BBsToProcess.push_back(Child);
            auto ChildBB = Child->getBlock();

            // 在父类中定义的值添加到当前子类的 RIVs 中。
            ResultMap[ChildBB].insert(ParentDefinedValues.begin(), ParentDefinedValues.end());

            // 将父类的 RIVs 添加到当前子类的 RIVs 中。
            ResultMap[ChildBB].insert(ParentRIVs.begin(), ParentRIVs.end());
        } // end for
    } // end while
    return ResultMap;
}

RIV::Result RIV::run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
    DominatorTree *DT = &FAM.getResult<DominatorTreeAnalysis>(F);
    Result Res = buildRIV(F, DT->getRootNode());
    return Res;
}

PreservedAnalyses
RIVPrinter::run(Function &F, FunctionAnalysisManager &FAM) {
    auto RIVMap = FAM.getResult<RIV>(F);
    printRIVResult(OS, RIVMap);
    return PreservedAnalyses::all();
}

// Legacy
bool LegacyRIV::runOnFunction(llvm::Function &F) {
    // 从前一个 runs 清除结果。
    RIVMap.clear();
    // 获得输入函数的 CFG 的入口节点。
    NodeTy Root = getAnalysis<DominatorTreeWrapperPass>().getDomTree().getRootNode();
    RIVMap = Impl.buildRIV(F, Root);
    return false;
}

void LegacyRIV::print(raw_ostream &out, Module const *) const {
    printRIVResult(out, RIVMap);
}

// 这个方法定义了这个 pass 是怎么和其他 pass 交互的。
void LegacyRIV::getAnalysisUsage(AnalysisUsage &AU) const {
    // 这个 pass 只依赖于 DominatorTree。
    AU.addRequired<DominatorTreeWrapperPass>();
    // 我们不更改其他 pass 的结果。
    AU.setPreservesAll();
}

// 注册
AnalysisKey RIV::Key;

llvm::PassPluginLibraryInfo getRIVPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "riv", LLVM_VERSION_STRING, [](PassBuilder &PB) {
        // #1 给 "opt -passes=print<riv>" 注册
        PB.registerPipelineParsingCallback(
            [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                if (Name == "print<riv>"){
                    FPM.addPass(RIVPrinter(llvm::errs()));
                    return true;
                }
                return false;
            }
        );

        // #2 给 ”FAM.getResult<RIV>(Function)" 注册
        PB.registerAnalysisRegistrationCallback(
            [](FunctionAnalysisManager &FAM) {
                FAM.registerPass([&] { return RIV(); });
            }
        );
    }};
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getRIVPluginInfo();
}

// Legacy PM 注册
char LegacyRIV::ID = 0;

// #1 注册 "opt -analyze -legacy-riv"
static RegisterPass<LegacyRIV> X("legacy-riv", "Compute Reachable Integer Values", true, true);

// 帮助函数
static void printRIVResult(raw_ostream &OutS, const RIV::Result &RIVMap) {
    OutS << "======================================\n";
    OutS << "RIV Analysis Result:\n";
    OutS << "======================================\n";

    const char *Str1 = "BB id";
    const char *Str2 = "Reachable Integer Values";
    OutS << format("%-10s %-30s\n", Str1, Str2);
    OutS << "--------------------------------------\n";

    const char *EmptyStr = "";

    for (auto const &KV : RIVMap) {
        std::string DummyStr;
        raw_string_ostream BBIdStream(DummyStr);
        KV.first->printAsOperand(BBIdStream, false);
        OutS << format("BB %-12s %-30s\n", BBIdStream.str().c_str(), EmptyStr);
        for (auto const *IntegerValue : KV.second) {
            std::string DummyStr;
            raw_string_ostream InstrStr(DummyStr);
            IntegerValue->print(InstrStr);
            OutS << format("%-12s %-30s\n", EmptyStr, InstrStr.str().c_str());
        }
    }
    OutS << "\n\n";
}



                





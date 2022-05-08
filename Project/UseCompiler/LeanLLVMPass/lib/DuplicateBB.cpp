/*

说明：

对于输入函数 F，DuplicateBB 首先创建一个适合克隆的基本块列表，然后克隆他们。如果一个基本块 BB 的 RIV 的集合是非空的，那么 BB 就会被克隆。

克隆会创建四个新的基本块来替代 BB。
lt-clone-1 和 lt-clone-2 BB 的克隆。
lt-if-then-else 包含 lt-then-else 语句用来决定两个克隆中的哪一个是正确的。
lt-tail 包含 PHI 节点，用来合并 lt-clone-1 和 lt-clone-2。 这个 if-then-else 基本块包含一个相当于以下伪代码的 IR

if (var == 0)
    goto BB-if-then
else
    goto BB-else

var 是一个从 BB 的 RIV 集合中随机选择的变量。如果 var 恰好是一个 GlobalValue，也就是全局变量，那么 BB 就不会被重复。这是因为全局变量是常量，而常量值会导致琐碎的 if 条件，比如 if (0 == 0)。

所有新创建的基本块都以原始基本块的数字 ID 作为后缀。

算法：

下面的 CFG 图表示应用 DuplicateBB 之前和之后的函数 F。假设 F 没有参数，所以只有一个入口基本块。[ BB1 ]和[ BB2 ]适合克隆。

--------------------------------------------------------------------------
F - BEFORE     (equivalence)          F - AFTER
--------------------------------------------------------------------------
[ entry ]                              [ entry ]                         |
    |                                      |                             |
    v            _________                 v                             e
    |           |                  [ if-then-else-1 ]                    x
    |           |                         / \                            e
    |           |                       /     \                          c
    |           |          [ lt-clone-1-1 ] [ lt-clone-1-2 ]             u
 [ BB1 ]       <                        \     /                          t
    |           |                         \ /                            i
    |           |                          v                             o
    |           |                    [ lt-tail-1 ]                       n
    |           |_________                 |                             |
    v            _________                 v                             d
    |           |                 [ lt-if-then-else-2 ]                  i
    |           |                         / \                            r
    |           |                       /     \                          e
    |           |          [ lt-clone-2-1 ] [ lt-clone-2-2 ]             c
 [ BB2 ]       <                        \     /                          t
    |           |                         \ /                            i
    |           |                          v                             o
    |           |                   [ lt-tail-2 ]                        n
    |           |_________                 |                             |
    v                                      v                             |
  (...)                                  (...)                           V
--------------------------------------------------------------------------

使用方法：

1. Legacy Pass 管理：
$ opt -load <BUILD_DIR>/lib/libRIV.so  -load <BUILD_DIR>/lib/libDuplicateBB%shlibext -legacy-duplicate-bb -S <bitcode-file>

2. New Pass 管理：
$ opt -load-pass-plugin <BUILD_DIR>/lib//libRIV.so -load-pass-plugin <BUILD_DIR>/lib//libDuplicateBB.so -passes=duplicate-bb -S <bitcode-file>

参考：
"Building, Testing and Debugging a Simple out-of-tree LLVM Pass", Serge Guelton and Adrien Guinet, LLVM Dev Meeting 2015

*/

#include "DuplicateBB.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <random>

#define DEBUG_TYPE "duplicate-bb"

STATISTIC(DuplicateBBCountStats, "The # of duplicated blocks");

using namespace llvm;

// DuplicateBB 实现
DuplicateBB::BBToSingleRIVMap
DuplicateBB::findBBsToDuplicate(Function &F, const RIV::Result &RIVResult) {
    BBToSingleRIVMap BlocksToDuplicate;

    // 获得一个随机数生成器。将会用在给注入的 if-then-else 结构选择一个上下文的值。llvm 11 之后，随机数生成器已经被移除。可以替换成 F.getParent()->createRNG("DuplicateBB")。https://reviews.llvm.org/rG73713f3e5ef2ecf1e5afafa89f76ab89cc06b18e
    std::random_device RD;
    std::mt19937 RNG(RD());

    for (BasicBlock &BB : F) {
        // 作为着陆点的基础块是用来处理异常的。暂不考虑。
        if (BB.isLandingPad()) {
            continue;
        }

        // 获得这个块的一套 RIVs。
        auto const &ReachableValues = RIVResult.lookup(&BB);
        size_t ReachableValuesCount = ReachableValues.size();

        // 这个 BB 有多少个 RIVs？我们至少需要一个可以复刻这个 BB。
        if (0 == ReachableValuesCount) {
            LLVM_DEBUG(errs() << "这个 BB 没有上下文值\n");
            continue;
        }

        // 从 RIVs 集合中随机选择一个上下文值。
        auto Iter = ReachableValues.begin();
        std::uniform_int_distribution<> Dist(0, ReachableValuesCount - 1);
        std::advance(Iter, Dist(RNG));

        if (dyn_cast<GlobalValue>(*Iter)) {
            LLVM_DEBUG(errs() << "这个 BB 中的上下文值是一个全局值. 我们跳过这个 BB\n");
            continue;
        }

        LLVM_DEBUG(errs() << "随机上下文值时：" << *Iter << "\n");

        // 存储当前 BB 和上下文变量之间的绑定，改变量将被用于 if-then-else 结构
        BlocksToDuplicate.emplace_back(&BB, *Iter);
    }
    return BlocksToDuplicate;
}

void DuplicateBB::cloneBB(BasicBlock &BB, Value *ContextValue, ValueToPhiMap &ReMapper) {
    // 不要复刻 Phi 节点 - 紧随其后
    Instruction *BBHead = BB.getFirstNonPHI();

    // 创建 if-then-else 条件分支
    IRBuilder<> Builder(BBHead);
    Value *Cond = Builder.CreateIsNull(ReMapper.count(ContextValue) ? ReMapper[ContextValue] : ContextValue);

    // 创建并插入 if-else 块。在这一点上，两个块都是微不足道的，只包含一条终止指令，分支到 BB 的尾部，其中包含从 BBHead 开始的所有指令。
    Instruction *ThenTerm = nullptr;
    Instruction *ElseTerm = nullptr;
    SplitBlockAndInsertIfThenElse(Cond, &*BBHead, &ThenTerm, &ElseTerm);
    BasicBlock *Tail = ThenTerm->getSuccessor(0);

    assert(Tail == ElseTerm->getSuccessor(0) && "不一致的 CFG");

    // 给一个新的块起个有意义的名字，不是必须的，但可以让输出更加可读。
    std::string DuplicatedBBId = std::to_string(DuplicateBBCount);
    ThenTerm->getParent()->setName("lt-clone-1-" + DuplicatedBBId);
    ElseTerm->getParent()->setName("lt-clone-2-" + DuplicatedBBId);
    Tail->setName("lt-tail-" + DuplicatedBBId);
    ThenTerm->getParent()->getSinglePredecessor()->setName("lt-if-then-else-" + DuplicatedBBId);

    // 用于跟踪新绑定的变量
    ValueToValueMapTy TailVMap, ThenVMap, ElseVMap;

    // Tail 不产生任何数值的指令列表，因此可以删除它。
    SmallVector<Instruction *, 8> ToRemove;

    // 遍历原始基本块，将每条指令复刻到 if-then 和 else 分支。及时更新绑定使用（通过 ThenVMap、ElseVMap、TailVMap）。在这个阶段，除了 PHI 节点外，所有的指令都存储在 Tail 中。
    for (auto IIT = Tail->begin(), IE = Tail->end(); IIT != IE; ++IIT) {
        Instruction &Instr = *IIT;
        assert(!isa<PHINode>(&Instr) && "Phi 节点已经过滤了");

        // 跳过终止符 -- 重复他们是没有意义的，除非我们要彻底删除 Tail
        if (Instr.isTerminator()) {
            RemapInstruction(&Instr, TailVMap, RF_IgnoreMissingLocals);
            continue;
        }

        // 复刻指令
        Instruction *ThenClone = Instr.clone(), *ElseClone = Instr.clone();

        // ThenClone 的操作数仍然持有对原始 BB 的引用，因此我们需要更新重新映射他们。
        RemapInstruction(ElseClone, ElseVMap, RF_IgnoreMissingLocals);
        ElseClone->insertBefore(ElseTerm);
        ElseVMap[&Instr] = ElseClone;

        // 不产生数值的指令可以安全的从 Tail 里删掉。
        if (ThenClone->getType()->isVoidTy()) {
            ToRemove.push_back(&Instr);
            continue;
        }

        // 产生值的指令不应该需要 Tail 里一个槽位。但是他们可以用于上下文，因此只要总是产生一个 PHI，并让后面的优化进行清理。
        PHINode *Phi = PHINode::Create(ThenClone->getType(), 2);
        Phi->addIncoming(ThenClone, ThenTerm->getParent());
        Phi->addIncoming(ElseClone, ElseTerm->getParent());
        TailVMap[&Instr] = Phi;

        ReMapper[&Instr] = Phi;

        // 指令是边走边修改的，使用 ReplaceInstWithInst 的迭代器版本。
        ReplaceInstWithInst(Tail->getInstList(), IIT, Phi);
    }

    // 清除不产生任何价值的指令。
    for (auto *I : ToRemove) {
        I->eraseFromParent();
    }

    ++DuplicateBBCount;
}

PreservedAnalyses DuplicateBB::run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
    BBToSingleRIVMap Targets = findBBsToDuplicate(F, FAM.getResult<RIV>(F));

    // 这个映射用于跟踪新的绑定。不然，来自 RIV 的信息将会过时。
    ValueToPhiMap ReMapper;

    // 复制
    for (auto &BB_Ctx : Targets) {
        cloneBB(*std::get<0>(BB_Ctx), std::get<1>(BB_Ctx), ReMapper);
    }

    DuplicateBBCountStats = DuplicateBBCount;
    return (Targets.empty() ? llvm::PreservedAnalyses::all() : llvm::PreservedAnalyses::none());
}

bool LegacyDuplicateBB::runOnFunction(llvm::Function &F) {
    // 查找要复制的 BB
    DuplicateBB::BBToSingleRIVMap Targets = Impl.findBBsToDuplicate(F, getAnalysis<LegacyRIV>().getRIVMap());

    // 这个映射用于跟踪新的绑定。不然，来自 RIV 的信息将会过时。
    DuplicateBB::ValueToPhiMap ReMapper;

    // 复制
    for (auto &BB_Ctx : Targets) {
        Impl.cloneBB(*std::get<0>(BB_Ctx), std::get<1>(BB_Ctx), ReMapper);
    }

    DuplicateBBCountStats = Impl.DuplicateBBCount;
    return (Targets.empty() ? false : true);
}

// New PM 注册
llvm::PassPluginLibraryInfo getDuplicateBBPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "duplicate-bb", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                        [](StringRef Name, FunctionPassManager &FPM,
                            ArrayRef<PassBuilder::PipelineElement>) {
                            if (Name == "duplicate-bb") {
                                FPM.addPass(DuplicateBB());
                                return true;
                            }
                            return false;
                        }); // end PB.registerPipelineParsingCallback
            }}; // end return
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getDuplicateBBPluginInfo();
}

// Legacy PM 注册
// 这个方法定义了这个 pass 是怎么和其它 pass 交互的。 
void LegacyDuplicateBB::getAnalysisUsage(AnalysisUsage &Info) const {
    Info.addRequired<LegacyRIV>();
}

char LegacyDuplicateBB::ID = 0;
static RegisterPass<LegacyDuplicateBB> X("legacy-duplicate-bb", "Duplicate Pass", false, false);




    










    

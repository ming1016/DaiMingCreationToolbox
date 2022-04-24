//==============================================================================
// FILE:
//    ConvertFCmpEq.h
//
// DESCRIPTION:
//    Declares the ConvertFCmpEq pass for the new and the legacy pass managers.
//
// License: MIT
//==============================================================================

#ifndef LLVM_TUTOR_CONVERT_FCMP_EQ_H
#define LLVM_TUTOR_CONVERT_FCMP_EQ_H

#include "FindFCmpEq.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

// Forward declarations
namespace llvm {

class Function;

} // namespace llvm

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------

struct ConvertFCmpEq : llvm::PassInfoMixin<ConvertFCmpEq> {
  // This is one of the standard run() member functions expected by
  // PassInfoMixin. When the pass is executed by the new PM, this is the
  // function that will be called.
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &FAM);
  // This is a helper run() member function overload which can be called by the
  // legacy pass (or any other code) without having to supply a
  // FunctionAnalysisManager argument.
  bool run(llvm::Function &Func, const FindFCmpEq::Result &Comparisons);
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct ConvertFCmpEqWrapper : llvm::FunctionPass {
  static char ID;
  ConvertFCmpEqWrapper();

  bool runOnFunction(llvm::Function &Func) override;
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};

#endif // !LLVM_TUTOR_CONVERT_FCMP_EQ_H

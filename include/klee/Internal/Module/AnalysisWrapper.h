//===--- AnalysisWrapper.h ------------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef ANALYSISWRAPPER_H_
#define ANALYSISWRAPPER_H_

#include "llvm/DebugInfo.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/AliasAnalysis.h"

#if LLVM_VERSION_CODE > LLVM_VERSION(3, 2)
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"
#else
#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/GlobalVariable.h"
#endif
#include "llvm/Support/raw_ostream.h"

#include <set>

namespace klee {
struct AnalysisWrapper : public llvm::ModulePass {
  static char ID;

  AnalysisWrapper() : llvm::ModulePass(ID) {}

  virtual bool runOnModule(llvm::Module &m);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};

static llvm::RegisterPass<AnalysisWrapper>
X("analysis-wrapper", "Calls all necessary LLVM analyses and transforms");
}
#endif /* ANALYISWRAPPER_H_ */

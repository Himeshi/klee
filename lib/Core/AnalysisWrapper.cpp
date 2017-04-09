//===------- AnalysisWrapper.cpp ------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Config/Version.h"
#include "klee/Internal/Module/AnalysisWrapper.h"

#define DEBUG_TYPE "analysis-wrapper"

#include "llvm/DebugInfo.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"

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

using namespace klee;

bool AnalysisWrapper::runOnModule(llvm::Module &m) {
  const llvm::LoopInfo &LI = getAnalysis<llvm::LoopInfo>();
  llvm::ScalarEvolution &SE = getAnalysis<llvm::ScalarEvolution>();

  for (llvm::Module::iterator func = m.begin(), fe = m.end(); func != fe;
       ++func) {
    for (llvm::Function::iterator bb = func->begin(), be = func->end();
         bb != be; ++bb) {
      const llvm::Loop *l = LI.getLoopFor(bb);
      if (l) {
        llvm::errs() << "Trip count: " << SE.getBackedgeTakenCount(l);
      }
    }
  }
  return false;
}

void AnalysisWrapper::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.addRequiredTransitive<llvm::ScalarEvolution>();
  AU.addRequiredTransitive<llvm::LoopInfo>();
  AU.addPreserved<llvm::ScalarEvolution>();
  AU.addPreserved<llvm::LoopInfo>();
}

char AnalysisWrapper::ID = 0;

static llvm::RegisterPass<AnalysisWrapper>
X("analysis-wrapper", "Calls all necessary LLVM analyses and transforms");

//===------- AnalysisWrapper.cpp ------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../../include/klee/Internal/Module/TripCounter.h"

#include "klee/Config/Version.h"

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

bool TripCounter::runOnModule(llvm::Module &m) {
  for (llvm::Module::iterator func = m.begin(), fe = m.end(); func != fe;
       ++func) {

    if (func->isDeclaration())
      continue;

    const llvm::LoopInfo &LI = getAnalysis<llvm::LoopInfo>(*func);
    llvm::ScalarEvolution &SE = getAnalysis<llvm::ScalarEvolution>(*func);

    for (llvm::Function::iterator bb = func->begin(), be = func->end();
         bb != be; ++bb) {
      const llvm::Loop *l = LI.getLoopFor(bb);
      if (l) {
        llvm::errs() << "Trip count: ";
        SE.getBackedgeTakenCount(l)->print(llvm::errs());
        llvm::errs() << "\n";
        llvm::errs() << "Loop:\n";
        l->print(llvm::errs());
      }
    }
  }
  return false;
}

void TripCounter::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<llvm::LoopInfo>();
  AU.addRequired<llvm::ScalarEvolution>();
}

char TripCounter::ID = 0;

static llvm::RegisterPass<TripCounter>
X("analysis-wrapper", "Calls all necessary LLVM analyses and transforms");

//===------- TripCounter.cpp ----------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../../include/klee/Internal/Module/TripCounter.h"

#define DEBUG_TYPE "trip-counter"

#include "llvm/DebugInfo.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

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

void TripCounter::analyzeSubLoops(llvm::ScalarEvolution &se,
                                  const llvm::Loop *l) {
  const std::vector<llvm::Loop *> &v = l->getSubLoops();
  const llvm::SCEV *scev = se.getBackedgeTakenCount(l);
  if (const llvm::SCEVConstant *scevConstant =
          llvm::dyn_cast<llvm::SCEVConstant>(scev)) {
    tripCount[l->getHeader()] = scevConstant->getValue()->getSExtValue();
  }

  for (std::vector<llvm::Loop *>::const_iterator it = v.begin(), ie = v.end();
       it != ie; ++it) {
    analyzeSubLoops(se, *it);
  }
}

bool TripCounter::getTripCount(llvm::BasicBlock *bb, int64_t &count) const {
  std::map<llvm::BasicBlock *, int64_t>::iterator it = tripCount.find(bb);
  if (it != tripCount.end()) {
    count = it->second;
    return true;
  }
  return false;
}

bool TripCounter::runOnModule(llvm::Module &m) {
  for (llvm::Module::iterator func = m.begin(), fe = m.end(); func != fe;
       ++func) {

    if (func->isDeclaration())
      continue;

    const llvm::LoopInfo &LI = getAnalysis<llvm::LoopInfo>(*func);
    llvm::ScalarEvolution &SE = getAnalysis<llvm::ScalarEvolution>(*func);

    for (llvm::LoopInfo::iterator it = LI.begin(), ie = LI.end(); it != ie;
         ++it) {
      analyzeSubLoops(SE, *it);
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

//===--- TripCounter.h ----------------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef TRIPCOUNTER_H_
#define TRIPCOUNTER_H_

#include "klee/Config/Version.h"

#include "llvm/DebugInfo.h"
#include "llvm/Pass.h"
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

#include <map>

namespace klee {

/// \brief A wrapper to LLVM analyses for computing loop trip counts.
///
/// References:
/// http://lists.llvm.org/pipermail/llvm-dev/2011-March/038502.html
/// https://groups.google.com/forum/#!topic/llvm-dev/1oNNBPMSqBg
struct TripCounter : public llvm::ModulePass {
  static char ID;

private:
  std::map<llvm::BasicBlock *, int64_t> tripCount;

  void analyzeSubLoops(llvm::ScalarEvolution &se, const llvm::Loop *l);

public:
  TripCounter() : llvm::ModulePass(ID) {}

  bool getTripCount(llvm::BasicBlock *bb, int64_t &count) const;

  virtual bool runOnModule(llvm::Module &m);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};
}
#endif /* TRIPCOUNTER_H_ */

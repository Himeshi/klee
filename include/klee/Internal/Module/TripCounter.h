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
#include <set>

namespace klee {

/// \brief A wrapper to LLVM analyses for computing loop trip counts.
///
/// References:
/// http://lists.llvm.org/pipermail/llvm-dev/2011-March/038502.html
/// https://groups.google.com/forum/#!topic/llvm-dev/1oNNBPMSqBg
struct TripCounter : public llvm::ModulePass {
  static char ID;

  static TripCounter *instance;

private:
  std::map<llvm::Instruction *, int64_t> tripCount;

  std::map<llvm::BasicBlock *, llvm::Instruction *> blockToFirstInstruction;

  std::map<llvm::Instruction *, llvm::BasicBlock *> exitBlock;

  std::map<llvm::Instruction *, llvm::Instruction *> firstInstructionOfExit;

  std::set<llvm::BasicBlock *> headerBlocks;

  std::set<llvm::Instruction *> realFirstInstruction;

  void analyzeSubLoops(llvm::ScalarEvolution &se, const llvm::Loop *l);

public:
  TripCounter() : llvm::ModulePass(ID) {}

  /// \brief Retrieve the trip count of the loop this instruction is in.
  ///
  /// \param inst the instruction for which its enclosing loop trip count is to
  /// be retrieved.
  /// \param count the loop trip count, a negative value if the instruction is
  /// not enclosed in a loop.
  /// \param exit the exit block
  ///
  /// \return true if inst was the first instruction of the loop header block
  bool getTripCount(llvm::Instruction *inst, int64_t &count,
                    llvm::BasicBlock *&exit) const;

  /// \brief Given the first instruction in the exit block, retrieve the first
  /// instruction in the loop body. This member function is used to indicate
  /// that the loop has been exited before the loop breaking routine is
  /// triggered.
  llvm::Instruction *getFirstInstructionOfExit(llvm::Instruction *inst) const {
    std::map<llvm::Instruction *, llvm::Instruction *>::const_iterator it =
        firstInstructionOfExit.find(inst);
    if (it != firstInstructionOfExit.end()) {
      return it->second;
    }
    return 0;
  }

  /// \brief Tests if the instruction is in header block
  bool isInHeaderBlock(llvm::Instruction *instr) const {
    if (llvm::BasicBlock *b = instr->getParent()) {
      return headerBlocks.find(b) != headerBlocks.end();
    }
    return false;
  }

  bool isRealFirstInstruction(llvm::Instruction *instr) const {
    return realFirstInstruction.find(instr) != realFirstInstruction.end();
  }

  virtual bool runOnModule(llvm::Module &m);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};
}
#endif /* TRIPCOUNTER_H_ */

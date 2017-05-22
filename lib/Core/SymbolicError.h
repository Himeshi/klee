//===-- SymbolicError.h ---------------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_SYMBOLICERROR_H_
#define KLEE_SYMBOLICERROR_H_

#include "ErrorState.h"

#include "klee/Expr.h"
#include "klee/util/ArrayCache.h"

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
#include "llvm/IR/Instructions.h"
#else
#include "llvm/Instructions.h"
#endif

namespace klee {
class Executor;

class SymbolicError {

  ref<ErrorState> errorState;

  std::map<llvm::Instruction *, uint64_t> nonExited;

public:
  SymbolicError() { errorState = ref<ErrorState>(new ErrorState()); }

  SymbolicError(SymbolicError &symErr)
      : errorState(symErr.errorState), nonExited(symErr.nonExited) {}

  ~SymbolicError();

  /// \brief Register the basic block if this basic block was a loop header
  bool addBasicBlock(llvm::Instruction *inst, llvm::BasicBlock *&exit);

  /// \brief Deregister the loop in nonExited if it is exited due to iteration
  /// numbers too small (< 2).
  void deregisterLoopIfExited(llvm::Instruction *inst);

  void outputErrorBound(llvm::Instruction *inst, double bound) {
    errorState->outputErrorBound(inst, bound);
  }

  ref<Expr> propagateError(Executor *executor, llvm::Instruction *instr,
                           ref<Expr> result,
                           std::vector<ref<Expr> > &arguments) {
    return errorState->propagateError(executor, instr, result, arguments);
  }

  ref<Expr> retrieveError(llvm::Value *value) {
    return errorState->retrieveError(value);
  }

  std::string &getOutputString() { return errorState->getOutputString(); }

  void executeStore(llvm::Instruction *inst, ref<Expr> address,
                    ref<Expr> error) {
    errorState->executeStore(inst, address, error);
  }

  ref<Expr> executeLoad(llvm::Value *value, ref<Expr> address) {
    return errorState->executeLoad(value, address);
  }

  /// print - Print the object content to stream
  void print(llvm::raw_ostream &os) const;

  /// dump - Print the object content to stderr
  void dump() const {
    print(llvm::errs());
    llvm::errs() << "\n";
  }
};
}

#endif /* KLEE_SYMBOLICERROR_H_ */

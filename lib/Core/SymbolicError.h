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
#include "klee/Internal/Module/Cell.h"
#include "klee/util/ArrayCache.h"

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
#include "llvm/IR/Instructions.h"
#else
#include "llvm/Instructions.h"
#endif

namespace klee {
class Executor;
class ExecutionState;
class KInstruction;

class SymbolicError {
  static uint64_t freshVariableId;

  ref<ErrorState> errorState;

  std::map<llvm::Instruction *, uint64_t> nonExited;

  /// \brief Record addresses used for writes to memory within each loop
  std::vector<std::map<ref<Expr>, ref<Expr> > > writesStack;

  /// \brief Record errors written into memory addresses within each loop
  std::vector<std::map<ref<Expr>, ref<Expr> > > initWritesErrorStack;

  /// \brief This data structure records the width of the results of phi
  /// instructions at the header block of a loop
  std::map<KInstruction *, unsigned int> phiResultWidthList;

  /// \brief This data structure records the initial
  std::vector<std::map<KInstruction *, ref<Expr> > > phiResultInitErrorStack;

  /// \brief Temporary PHI result initial error amount
  std::map<KInstruction *, ref<Expr> > tmpPhiResultInitError;

public:
  SymbolicError() { errorState = ref<ErrorState>(new ErrorState()); }

  SymbolicError(SymbolicError &symErr)
      : errorState(symErr.errorState), nonExited(symErr.nonExited),
        writesStack(symErr.writesStack),
        initWritesErrorStack(symErr.initWritesErrorStack),
        phiResultWidthList(symErr.phiResultWidthList),
        phiResultInitErrorStack(symErr.phiResultInitErrorStack),
        tmpPhiResultInitError(symErr.tmpPhiResultInitError) {}

  ~SymbolicError();

  /// \brief Compute the error introduced by the loop's computation, which
  /// is (initError + (tripCount - 1) * (endError - initError)). initError
  /// is the error at the first iteration of the loop, while endError is
  /// the error at the second iteration of the loop (e.g., between same target
  /// store instructions).
  static ref<Expr> computeLoopError(int64_t tripCount, ref<Expr> initError,
                                    ref<Expr> endError);

  /// \brief Register the basic block if this basic block was a loop header
  bool breakLoop(Executor *executor, ExecutionState &state,
                 llvm::Instruction *inst, llvm::BasicBlock *&exit);

  /// \brief Create a read expression of a fresh variable
  ref<Expr> createFreshRead(Executor *executor, ExecutionState &state,
                            unsigned int width);

  /// \brief Deregister the loop in nonExited if it is exited due to iteration
  /// numbers too small (< 2).
  void deregisterLoopIfExited(Executor *executor, ExecutionState &state,
                              llvm::Instruction *inst);

  void outputErrorBound(llvm::Instruction *inst, double bound) {
    errorState->outputErrorBound(inst, bound);
  }

  ref<Expr> propagateError(Executor *executor, KInstruction *ki,
                           ref<Expr> result, std::vector<ref<Expr> > &arguments,
                           unsigned int phiResultWidth = 0);

  ref<Expr> retrieveError(llvm::Value *value) {
    return errorState->retrieveError(value);
  }

  std::string &getOutputString() { return errorState->getOutputString(); }

  void executeStore(llvm::Instruction *inst, ref<Expr> address, ref<Expr> value,
                    ref<Expr> error);

  void storeError(llvm::Instruction *inst, ref<Expr> address, ref<Expr> error) {
    errorState->executeStoreSimple(inst, address, error);
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

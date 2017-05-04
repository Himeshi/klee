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

  std::vector<ref<ErrorState> > errorStateStack;

  std::map<llvm::Instruction *, uint64_t> nonExited;

public:
  SymbolicError() {
    ref<ErrorState> ret(new ErrorState());
    errorStateStack.push_back(ret);
  }

  SymbolicError(SymbolicError &symErr)
      : errorStateStack(symErr.errorStateStack), nonExited(symErr.nonExited) {}

  ~SymbolicError();

  /// \brief Register the basic block if this basic block was a loop header
  bool addBasicBlock(llvm::Instruction *inst);

  void outputErrorBound(llvm::Instruction *inst, double bound) {
    errorStateStack.back()->outputErrorBound(inst, bound);
  }

  ref<Expr> propagateError(Executor *executor, llvm::Instruction *instr,
                           ref<Expr> result,
                           std::vector<ref<Expr> > &arguments) {
    return errorStateStack.back()->propagateError(executor, instr, result,
                                                  arguments);
  }

  ref<Expr> retrieveError(llvm::Value *value) {
    return errorStateStack.back()->retrieveError(value);
  }

  std::string &getOutputString() {
    return errorStateStack.back()->getOutputString();
  }

  void executeStore(ref<Expr> address, ref<Expr> error) {
    return errorStateStack.back()->executeStore(address, error);
  }

  ref<Expr> executeLoad(llvm::Value *value, ref<Expr> address) {
    return errorStateStack.back()->executeLoad(value, address);
  }

  /// print - Print the object content to stream
  void print(llvm::raw_ostream &os) const { errorStateStack.back()->print(os); }

  /// dump - Print the object content to stderr
  void dump() const {
    print(llvm::errs());
    llvm::errs() << "\n";
  }
};
}

#endif /* KLEE_SYMBOLICERROR_H_ */

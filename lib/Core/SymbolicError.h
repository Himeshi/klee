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

  std::map<llvm::Value *, ref<Expr> > valueErrorMap;

  std::map<const Array *, const Array *> arrayErrorArrayMap;

  ref<Expr> getError(Executor *executor, ref<Expr> valueExpr,
                     llvm::Value *value = 0);

  ArrayCache errorArrayCache;

  std::string outputString;

  std::map<uintptr_t, ref<Expr> > storedError;

  std::map<llvm::BasicBlock *, uint64_t> nonExited;

  llvm::BasicBlock *lastBasicBlock;

public:
  SymbolicError() : lastBasicBlock(0) {}

  SymbolicError(SymbolicError &symErr)
      : nonExited(symErr.nonExited), lastBasicBlock(symErr.lastBasicBlock) {
    storedError = symErr.storedError;
    // FIXME: Simple copy for now.
    valueErrorMap = symErr.valueErrorMap;
  }

  ~SymbolicError();

  /// \brief Register the basic block if this basic block was a loop header
  bool addBasicBlock(llvm::Instruction *inst);

  void outputErrorBound(llvm::Instruction *inst, double bound);

  ref<Expr> propagateError(Executor *executor, llvm::Instruction *instr,
                           ref<Expr> result,
                           std::vector<ref<Expr> > &arguments);

  std::string getOutputString() { return outputString; }

  void executeStore(ref<Expr> address, ref<Expr> error);

  ref<Expr> executeLoad(llvm::Value *value, ref<Expr> address);

  /// print - Print the object content to stream
  void print(llvm::raw_ostream &os) const;

  /// dump - Print the object content to stderr
  void dump() const { print(llvm::errs()); }
};
}

#endif /* KLEE_SYMBOLICERROR_H_ */

//===-- SymbolicError.cpp -------------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "SymbolicError.h"

#include "klee/Config/Version.h"
#include "klee/Internal/Module/TripCounter.h"

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#else
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#endif

using namespace klee;

bool SymbolicError::addBasicBlock(llvm::Instruction *inst) {
  int64_t tripCount;
  if (TripCounter::instance &&
      TripCounter::instance->getTripCount(inst, tripCount)) {
    std::map<llvm::Instruction *, uint64_t>::iterator it = nonExited.find(inst);

    bool ret = (it != nonExited.end() && it->second > 0);
    if (ret) {
      errorStateStack.pop_back();
      --(it->second);
    } else {
      ref<ErrorState> newErrorState(new ErrorState());
      errorStateStack.push_back(newErrorState);
      nonExited[inst] = 1;
    }
    return ret;
  }
  return false;
}

SymbolicError::~SymbolicError() {
  nonExited.clear();
}

void SymbolicError::print(llvm::raw_ostream &os) const {
  os << "----------------------------------------------\n";
  os << "SYMBOLIC ERROR STACK (" << errorStateStack.size() << " ELEMENT(S))";
  for (std::vector<ref<ErrorState> >::const_reverse_iterator
           it = errorStateStack.rbegin(),
           is = it, ie = errorStateStack.rend();
       it != ie; ++it) {
    os << "\n----------------------------------------------\n";
    (*it)->print(os);
  }
}

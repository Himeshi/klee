//===-- LoopDetector.cpp --------------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "LoopDetector.h"

using namespace klee;

bool LoopDetector::addBasicBlock(llvm::BasicBlock *bb) {
  if (lastBasicBlock == bb)
    return false;

  lastBasicBlock = bb;

  std::map<llvm::BasicBlock *, uint64_t>::iterator it = nonExited.find(bb);

  bool ret = (it != nonExited.end() && it->second > 0);
  if (ret) {
    --(it->second);
  } else {
    nonExited[bb] = 1;
  }
  return ret;
}

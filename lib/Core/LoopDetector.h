//===-- LoopDetector.h ----------------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_LOOPDETECTOR_H_
#define KLEE_LOOPDETECTOR_H_

#include "klee/Config/Version.h"

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#else
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#endif

#include <map>

namespace klee {

class LoopDetector {

  std::map<llvm::BasicBlock *, uint64_t> nonExited;

  llvm::BasicBlock *lastBasicBlock;

public:
  LoopDetector() : lastBasicBlock(0) {}

  LoopDetector(const LoopDetector &loopDetector)
      : nonExited(loopDetector.nonExited),
        lastBasicBlock(loopDetector.lastBasicBlock) {}

  ~LoopDetector() { nonExited.clear(); }

  bool addBasicBlock(llvm::BasicBlock *bb);
};
}

#endif /* KLEE_LOOPDETECTOR_H_ */

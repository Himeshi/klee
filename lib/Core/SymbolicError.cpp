//===-- SymbolicError.cpp -------------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "SymbolicError.h"

#include "klee/CommandLine.h"
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

bool SymbolicError::addBasicBlock(llvm::Instruction *inst,
                                  llvm::BasicBlock *&exit) {
  if (!LoopBreaking)
    return false;

  int64_t tripCount;
  if (TripCounter::instance &&
      TripCounter::instance->getTripCount(inst, tripCount, exit)) {
    // Loop is entered
    std::map<llvm::Instruction *, uint64_t>::iterator it = nonExited.find(inst);

    bool ret = (it != nonExited.end() && it->second > 0);
    if (ret) {
      --(it->second);
      if ((it->second) % 2 == 0) {
        // We are exiting the loop

        // Pop the last memory writes record
        writesStack.pop_back();

        // Erase the loop from not-yet-exited loops map
        nonExited.erase(it);
        return true;
      }
    } else {
      // Loop is entered for the first time

      // Add element to write record
      writesStack.push_back(std::set<uint64_t>());

      // Set the iteration reverse count.
      nonExited[inst] += 2;
    }
  }
  return false;
}

void SymbolicError::deregisterLoopIfExited(llvm::Instruction *inst) {
  llvm::Instruction *firstLoopInst =
      TripCounter::instance->getFirstInstructionOfExit(inst);
  std::map<llvm::Instruction *, uint64_t>::iterator it =
      nonExited.find(firstLoopInst);
  if (it != nonExited.end()) {
    // We are exiting the loop

    // Pop the last memory writes record
    writesStack.pop_back();

    // Erase the loop from not-yet-exited loops map
    nonExited.erase(it);
  }
}

SymbolicError::~SymbolicError() {
  nonExited.clear();
}

void SymbolicError::print(llvm::raw_ostream &os) const {
  errorState->print(os);

  os << "\nNon-Exited Loops:";
  if (!nonExited.empty()) {
    for (std::map<llvm::Instruction *, uint64_t>::const_iterator
             it = nonExited.begin(),
             ie = nonExited.end();
         it != ie; ++it) {
      os << "\nheader: ";
      it->first->print(os);
      os << ", iterations left: " << it->second;
    }
  } else {
    os << " (none)";
  }
}

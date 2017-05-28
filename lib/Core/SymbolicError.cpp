//===-- SymbolicError.cpp -------------------------------------------------===//
//
// The KLEE Symbolic Virtual Machine with Numerical Error Analysis Extension
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "SymbolicError.h"

#include "Executor.h"
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

uint64_t SymbolicError::freshVariableId = 0;

bool SymbolicError::addBasicBlock(Executor *executor, ExecutionState &state,
                                  llvm::Instruction *inst,
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

        for (std::map<ref<Expr>, ref<Expr> >::iterator
                 it1 = writesStack.back().begin(),
                 ie1 = writesStack.back().end();
             it1 != ie1; ++it1) {
          Cell addressCell;
          addressCell.value = it1->first;
          ref<Expr> error = errorState->retrieveStoredError(it1->first);
          ref<Expr> freshRead = executor->createFreshArray(
              state, SymbolicError::freshVariableId, it1->second->getWidth());
          executor->executeMemoryOperation(state, true, addressCell, freshRead,
                                           error, 0);
        }
        // Pop the last memory writes record
        writesStack.pop_back();

        // Erase the loop from not-yet-exited loops map
        nonExited.erase(it);
        return true;
      }
    } else {
      // Loop is entered for the first time

      // Add element to write record
      writesStack.push_back(std::map<ref<Expr>, ref<Expr> >());

      // Set the iteration reverse count.
      nonExited[inst] += 2;
    }
  }
  return false;
}

ref<Expr> SymbolicError::createFreshRead(Executor *executor,
                                         ExecutionState &state,
                                         unsigned int width) {
  return executor->createFreshArray(state, SymbolicError::freshVariableId,
                                    width);
}

void SymbolicError::deregisterLoopIfExited(Executor *executor,
                                           ExecutionState &state,
                                           llvm::Instruction *inst) {
  llvm::Instruction *firstLoopInst =
      TripCounter::instance->getFirstInstructionOfExit(inst);
  std::map<llvm::Instruction *, uint64_t>::iterator it =
      nonExited.find(firstLoopInst);
  if (it != nonExited.end()) {
    // We are exiting the loop

    for (std::map<ref<Expr>, ref<Expr> >::iterator
             it1 = writesStack.back().begin(),
             ie1 = writesStack.back().end();
         it1 != ie1; ++it1) {
      Cell addressCell;
      addressCell.value = it1->second;
      ref<Expr> error = errorState->retrieveStoredError(it1->first);
      ref<Expr> freshRead =
          createFreshRead(executor, state, it1->second->getWidth());
      executor->executeMemoryOperation(state, true, addressCell, freshRead,
                                       error, 0);
    }

    // Pop the last memory writes record
    writesStack.pop_back();

    // Erase the loop from not-yet-exited loops map
    nonExited.erase(it);
  }
}

SymbolicError::~SymbolicError() {
  nonExited.clear();
}

void SymbolicError::executeStore(llvm::Instruction *inst, ref<Expr> address,
                                 ref<Expr> value, ref<Expr> error) {
  if (LoopBreaking && !writesStack.empty()) {
    if (llvm::isa<ConstantExpr>(address)) {
      std::map<ref<Expr>, ref<Expr> > &writesMap = writesStack.back();
      writesMap[address] = value;
    } else {
      assert(!"non-constant address");
    }
  }
  storeError(inst, address, error);
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

  os << "\nWrites stack:";
  if (!writesStack.empty()) {
    for (std::vector<std::map<ref<Expr>, ref<Expr> > >::const_iterator
             it = writesStack.begin(),
             ie = writesStack.end();
         it != ie; ++it) {
      os << "\n-----------------------------";
      for (std::map<ref<Expr>, ref<Expr> >::const_iterator it1 = it->begin(),
                                                           ie1 = it->end();
           it1 != ie1; ++it1) {
        os << "\n[";
        it1->first->print(os);
        os << "] -> [";
        it1->second->print(os);
        os << "]";
      }
    }
  } else {
    os << " (empty)";
  }
}

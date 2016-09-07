/*
 * SymbolicError.h
 *
 *  Created on: 18 Aug 2016
 *      Author: himeshi
 */

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

		std::map<Expr *, ref<Expr> > valueErrorMap;

		std::map<const Array *, const Array *> arrayErrorArrayMap;

		ref<Expr> getError(Executor *executor, Expr *value);

		ArrayCache errorArrayCache;

	public:
		SymbolicError() {
		}

		~SymbolicError();

		void propagateError(Executor *executor, llvm::Instruction *instr,
				ref<Expr> result, std::vector<ref<Expr> > &arguments);
	};
}

#endif /* KLEE_SYMBOLICERROR_H_ */

/*
 * SymbolicError.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: himeshi
 */
#include "SymbolicError.h"
#include "llvm/IR/Instructions.h"

using namespace klee;

ref<Expr> SymbolicError::getError(Executor *executor, Expr *value) {
	ref<Expr> errorAmount = valueErrorMap[value];

	if (!errorAmount.get()) {
		if (llvm::isa<ReadExpr>(value)) {
			ReadExpr *readExpr = llvm::dyn_cast<ReadExpr>(value);
			const Array *readArray = readExpr->updates.root;
			const Array *errorArray = arrayErrorArrayMap[readArray];
			if (!errorArray) {
				std::string errorName(
						"__error__of__" + readExpr->updates.root->name);
				const Array *newErrorArray = errorArrayCache.CreateArray(
						errorName, Expr::Int32);
				UpdateList ul(newErrorArray, 0);
				arrayErrorArrayMap[readArray] = newErrorArray;
				ref<Expr> newReadExpr = ReadExpr::create(ul,
						ConstantExpr::alloc(0, Expr::Int32));
				valueErrorMap[value] = newReadExpr;
				return newReadExpr;
			}
			UpdateList ul(errorArray, 0);
			ref<Expr> newReadExpr = ReadExpr::create(ul,
					ConstantExpr::alloc(0, Expr::Int32));
			valueErrorMap[value] = newReadExpr;
			return newReadExpr;
		} else if (llvm::isa<ConstantExpr>(value)) {
			return ConstantExpr::alloc(0, Expr::Int32);
		} else {
			//assert(!"malformed expression");
		}
	}
	return errorAmount;
}

SymbolicError::~SymbolicError() {
	arrayErrorArrayMap.clear();
	valueErrorMap.clear();
}

void SymbolicError::propagateError(Executor *executor, llvm::Instruction *instr,
		ref<Expr> result, std::vector<ref<Expr> > &arguments) {
	switch (instr->getOpcode()) {
	case llvm::Instruction::Add:{
		ref<Expr> lError = valueErrorMap[arguments[0].get()];
		ref<Expr> rError = valueErrorMap[arguments[1].get()];
		if (!lError.get()) {
			lError = getError(executor, arguments[0].get());
		}
		if (!rError.get()) {
			rError = getError(executor, arguments[1].get());
		}
		valueErrorMap[result.get()] = AddExpr::create(lError, rError);
		break;
	}
	}
}

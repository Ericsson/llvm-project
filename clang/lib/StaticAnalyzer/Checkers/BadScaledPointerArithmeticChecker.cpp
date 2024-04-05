//===-- BadScaledPointerArithmetic.cpp ----------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines a checker that detects situations when an numerical value that
// involves sizeof or offsetof is added to (or subtracted from) a non-char
// pointer type
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/FormatVariadic.h"
#include <utility>

using namespace clang;
using namespace ento;
using llvm::formatv;

namespace {

class BadScaledPointerArithmeticChecker : public Checker<check::PreStmt<BinaryOperator>> {
  const BugType BT{this, "Badly scaled pointer arithmetic",
                                   "Suspicious operation"};

  void reportBug(bool IsLeft, CheckerContext &C) const;
  bool isCharPtr(QualType T, CheckerContext &C) const;
public:
  void checkPreStmt(const BinaryOperator *BO, CheckerContext &C) const;
};

} // end anonymous namespace

bool BadScaledPointerArithmeticChecker::isCharPtr(QualType T, CheckerContext &C) const {
  if (T.isNull() || !T->isAnyPointerType())
    return false;
  QualType PT = T->getPointeeType();
  if (PT.isNull() || PT->isIncompleteType() || PT->isDependentType() ||
      isa<DependentSizedArrayType>(PT) || !PT->isConstantSizeType())
    return false;
  return C.getASTContext().getTypeSizeInChars(PT).getQuantity() == 1;
}

void BadScaledPointerArithmeticChecker::checkPreStmt(const BinaryOperator *BO, CheckerContext &C) const {
  BinaryOperatorKind OpKind = BO->getOpcode();
  if (!BO->isAdditiveOp() && OpKind != BO_AddAssign && OpKind != BO_SubAssign)
    return;

  const Expr *Lhs = BO->getLHS();
  const Expr *Rhs = BO->getRHS();

  if (Lhs->getType()->isPointerType() && Rhs->getType()->isIntegerType()) {
    SVal RHSVal = C.getSVal(Rhs);
    if (RHSVal.isFromSizeof() && !isCharPtr(Lhs->getType(), C))
      reportBug(/*IsLeft=*/false, C);
  } else if (Lhs->getType()->isIntegerType() && Rhs->getType()->isPointerType()) {
    SVal LHSVal = C.getSVal(Lhs);
    if (LHSVal.isFromSizeof() && !isCharPtr(Rhs->getType(), C))
      reportBug(/*IsLeft=*/true, C);
  }
}
void BadScaledPointerArithmeticChecker::reportBug(bool IsLeft, CheckerContext &C) const {
  ExplodedNode *N = C.generateNonFatalErrorNode(C.getState());
  if (!N)
    return;
  std::string BugMessage = formatv("In pointer arithmetic {0} argument is calculated from a sizeof or offsetof expression", IsLeft ? "left" : "right");

  auto R = std::make_unique<PathSensitiveBugReport>(
      BT, BugMessage, N);
  C.emitReport(std::move(R));
}

void ento::registerBadScaledPointerArithmeticChecker(CheckerManager &mgr) {
  mgr.registerChecker<BadScaledPointerArithmeticChecker>();
}

// This checker should be enabled regardless of how language options are set.
bool ento::shouldRegisterBadScaledPointerArithmeticChecker(const CheckerManager &mgr) {
  return true;
}

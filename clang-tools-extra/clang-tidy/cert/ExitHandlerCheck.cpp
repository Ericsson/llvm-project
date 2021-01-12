//===--- ExitHandlerCheck.cpp - clang-tidy --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ExitHandlerCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include <deque>
#include <iterator>

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cert {

namespace {

/// The following functions are considered exit functions:
/// '_Exit'
/// 'exit'
/// 'quick_exit'
/// 'abort'
/// 'terminate'
/// But only if they are in the global or ::std namespace, with the exception of
/// terminate, which only exists in the std namespace.
bool isExitFunction(const FunctionDecl *FD) {
  const StringRef FN = FD->getName();
  const bool InStdNS = FD->isInStdNamespace();
  if (FN == "terminate" && InStdNS)
    return true;

  constexpr StringRef GlobalAndStdFunctions[] = {"_Exit", "exit", "quick_exit",
                                                 "abort"};
  const bool InGlobalNS = FD->isGlobal();
  return llvm::is_contained(GlobalAndStdFunctions, FN) &&
         (InGlobalNS || InStdNS);
}

/// Only global and ::std namespaced 'longjmp' functions are considered.
bool isLongJump(const FunctionDecl *FD) {
  return FD->getName() == "longjmp" &&
         (FD->isGlobal() || FD->isInStdNamespace());
}

class CalledFunctionsCollector
    : public RecursiveASTVisitor<CalledFunctionsCollector> {
  // The declarations and usages of encountered functions.
  llvm::SmallVector<std::pair<const FunctionDecl *, const Expr *>, 32>
      CalledFunctions;

public:
  bool VisitCallExpr(const CallExpr *CE) {
    if (const FunctionDecl *F = CE->getDirectCallee())
      CalledFunctions.emplace_back(F, CE);
    return true;
  }

  void clear() { CalledFunctions.clear(); }

  /// Iteration over the collector is iteration over the found FunctionDecls.
  /// In order to allow moving from the underlying container, non-const
  /// interators are allowed.
  auto begin() { return CalledFunctions.begin(); }
  auto end() { return CalledFunctions.end(); }
};

constexpr char HandlerDeclLabel[] = "handler_decl";
constexpr char HandlerExprLabel[] = "handler_expr";
constexpr char RegisterCallLabel[] = "register_call";

} // namespace

/// Match register-function calls, that has handler-functions as their first
/// argument.
void ExitHandlerCheck::registerMatchers(MatchFinder *Finder) {
  const auto IsRegisterFunction = callee(functionDecl(hasAnyName(
      "::atexit", "std::atexit", "::at_quick_exit", "std::at_quick_exit")));
  const auto HasHandlerAsFirstArg = hasArgument(
      0, declRefExpr(hasDeclaration(functionDecl().bind(HandlerDeclLabel)))
             .bind(HandlerExprLabel));
  Finder->addMatcher(callExpr(IsRegisterFunction, HasHandlerAsFirstArg)
                         .bind(RegisterCallLabel),
                     this);
}

void ExitHandlerCheck::reportExitFunction(const CallExpr *RegisterCall,
                                          const FunctionDecl *Handler,
                                          const Expr *Usage) {
  // An exit-function is encountered somewhere in the callgraph of the
  // handler.
  diag(RegisterCall->getBeginLoc(),
       "exit-handler potentially calls an exit function instead of terminating "
       "normally with a return");
  diag(Handler->getBeginLoc(), "handler function declared here",
       DiagnosticIDs::Note);
  diag(Usage->getBeginLoc(), "exit function called here", DiagnosticIDs::Note);
}

void ExitHandlerCheck::reportLongJump(const CallExpr *RegisterCall,
                                      const FunctionDecl *Handler,
                                      const Expr *Usage) {
  // A jump function is encountered somewhere in the callgraph of the
  // handler.
  diag(RegisterCall->getSourceRange().getBegin(),
       "exit-handler potentially calls a longjmp instead of terminating "
       "normally with a return");
  diag(Handler->getBeginLoc(), "handler function declared here",
       DiagnosticIDs::Note);
  diag(Usage->getBeginLoc(), "jump function called here", DiagnosticIDs::Note);
}

/// Check if the callgraph of the handler-function contains any exit functions
/// or jump functions.
void ExitHandlerCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *RegisterCall =
      Result.Nodes.getNodeAs<CallExpr>(RegisterCallLabel);
  const auto *HandlerDecl =
      Result.Nodes.getNodeAs<FunctionDecl>(HandlerDeclLabel);
  const auto *HandlerExpr =
      Result.Nodes.getNodeAs<DeclRefExpr>(HandlerExprLabel);

  // Visit each function encountered in the callgraph only once.
  llvm::DenseSet<const FunctionDecl *> SeenFunctions;

  // Reuse the ASTVistor instance that collects the called functions.
  CalledFunctionsCollector Collector;

  // The worklist of the callgraph visitation algorithm.
  std::deque<std::pair<const FunctionDecl *, const Expr *>> CalledFunctions{
      {HandlerDecl, HandlerExpr}};

  // Visit the definition of every function referenced by the handler function,
  // and look for exit-functions and jump calls.
  while (!CalledFunctions.empty()) {
    // Use the canonical declaration for uniquing.
    const FunctionDecl *Current =
        CalledFunctions.front().first->getCanonicalDecl();
    const Expr *CurrentUsage = CalledFunctions.front().second;
    CalledFunctions.pop_front();

    // Do not visit functions with same canonical declaration twice.
    if (!SeenFunctions.insert(Current).second)
      continue;

    if (isExitFunction(Current)) {
      reportExitFunction(RegisterCall, HandlerDecl, CurrentUsage);
      break;
    }
    if (isLongJump(Current)) {
      reportLongJump(RegisterCall, HandlerDecl, CurrentUsage);
      break;
    }

    // Get the body of the encountered non-exit and non-longjmp function.
    const FunctionDecl *CurrentDefWithBody;
    if (!Current->hasBody(CurrentDefWithBody))
      continue;

    // Reset the ASTVisitor instance results.
    Collector.clear();
    // Collect all the referenced FunctionDecls.
    Collector.TraverseStmt(CurrentDefWithBody->getBody());
    // Move the called functions to the worklist.
    llvm::move(Collector, std::back_inserter(CalledFunctions));
  }
}

} // namespace cert
} // namespace tidy
} // namespace clang

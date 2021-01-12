//===--- ExitHandlerCheck.h - clang-tidy ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CERT_EXITHANDLERCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CERT_EXITHANDLERCHECK_H

#include "../ClangTidyCheck.h"

namespace clang {
namespace tidy {
namespace cert {

/// Checker for SEI CERT rule ENV32-C
/// All exit handlers must return normally.
/// Exit handlers must terminate by returning. It is important and potentially
/// safety-critical for all exit handlers to be allowed to perform their cleanup
/// actions. This is particularly true because the application programmer does
/// not always know about handlers that may have been installed by support
/// libraries. Two specific issues include nested calls to an exit function and
/// terminating a call to an exit handler by invoking longjmp.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/cert-exit-handler-check.html
class ExitHandlerCheck : public ClangTidyCheck {
public:
  ExitHandlerCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  void reportExitFunction(const CallExpr *RegisterCall,
                          const FunctionDecl *Handler, const Expr *Usage);
  void reportLongJump(const CallExpr *RegisterCall, const FunctionDecl *Handler,
                      const Expr *Usage);
};

} // namespace cert
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CERT_EXITHANDLERCHECK_H

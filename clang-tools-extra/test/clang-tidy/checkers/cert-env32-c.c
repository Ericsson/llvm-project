// Test as a C file.
// RUN: %check_clang_tidy %s cert-env32-c %t -- -- -DCMODE
//
// Test as a C++ file.
//
// Test functions in global namespace.
// RUN: %check_clang_tidy -assume-filename=%s.cpp %s cert-env32-c %t \
// RUN:     -- -- -DCPPMODE
//
// Test functions in std namespace.
// RUN: %check_clang_tidy -assume-filename=%s.cpp %s cert-env32-c %t \
// RUN:     -- -- -DCPPMODE -DTEST_NS_NAME=std

#if defined(CPPMODE) && defined(TEST_NS_NAME)
namespace TEST_NS_NAME {
#endif

// --------------
// EXIT FUNCTIONS
// --------------

// No handlers are invoked when _Exit is called.
void _Exit(int __status);

// Handlers registered by atexit are invoked in reverse order when exit is
// called.
void exit(int __status);

// Handlers registered by at_quick_exit are invoked in reverse order when
// quick_exit is called.
void quick_exit(int __status);

// The program is terminated without destroying any object and without calling
// any of the functions passed to atexit or at_quick_exit.
void abort();

// --------------------
// HANDLER REGISTRATION
// --------------------

// Register handlers to run when exit is called.
int atexit(void (*__func)(void));

// Register handlers to run when exit is called.
int at_quick_exit(void (*__func)(void));

// --------------
// Setjmp/longjmp
// --------------
// C99 requires jmp_buf to be an array type.
typedef int jmp_buf[1];
int setjmp(jmp_buf);
void longjmp(jmp_buf, int);

// Compliant solutions

void cleanup1() {
  // do cleanup
}

void cleanup2() {
  // do cleanup
}

void test_atexit_single_compliant() {
  (void)atexit(cleanup1);
}

void test_atexit_multiple_compliant() {
  (void)atexit(cleanup1);
  (void)atexit(cleanup2);
}

void test_at_quick_exit_single_compliant() {
  (void)at_quick_exit(cleanup1);
}

void test_at_quick_exit_multiple_compliant() {
  (void)at_quick_exit(cleanup1);
  (void)at_quick_exit(cleanup2);
}

// Non-compliant solutions calling _Exit

void call__Exit() {
  _Exit(0);
}

void call_call__Exit() {
  call__Exit();
}

extern int unknown__Exit_flag;

void call__Exit_conditionally() {
  if (unknown__Exit_flag)
    call__Exit();
}

void call_call__Exit_conditionally() {
  call__Exit_conditionally();
}

void test__Exit_called_directly() {
  (void)atexit(call__Exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-22]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-22]]:3: note: exit function called here
  (void)at_quick_exit(call__Exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-26]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-26]]:3: note: exit function called here
};

void test__Exit_called_indirectly() {
  (void)atexit(call_call__Exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-29]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-33]]:3: note: exit function called here
  (void)at_quick_exit(call_call__Exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-33]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-37]]:3: note: exit function called here
};

void test_conditional__Exit_called_directly() {
  (void)atexit(call__Exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-34]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-44]]:3: note: exit function called here
  (void)at_quick_exit(call__Exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-38]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-48]]:3: note: exit function called here
};

void test_conditional__Exit_called_indirectly() {
  (void)atexit(call_call__Exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-40]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-55]]:3: note: exit function called here
  (void)at_quick_exit(call_call__Exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-44]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-59]]:3: note: exit function called here
};

// Non-compliant solutions calling exit

void call_exit() {
  exit(0);
}

void call_call_exit() {
  call_exit();
}

extern int unknown_exit_flag;

void call_exit_conditionally() {
  if (unknown_exit_flag)
    call_exit();
}

void call_call_exit_conditionally() {
  call_exit_conditionally();
}

void test_exit_called_directly() {
  (void)atexit(call_exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-22]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-22]]:3: note: exit function called here
  (void)at_quick_exit(call_exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-26]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-26]]:3: note: exit function called here
};

void test_exit_called_indirectly() {
  (void)atexit(call_call_exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-29]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-33]]:3: note: exit function called here
  (void)at_quick_exit(call_call_exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-33]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-37]]:3: note: exit function called here
};

void test_conditional_exit_called_directly() {
  (void)atexit(call_exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-34]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-44]]:3: note: exit function called here
  (void)at_quick_exit(call_exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-38]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-48]]:3: note: exit function called here
};

void test_conditional_exit_called_indirectly() {
  (void)atexit(call_call_exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-40]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-55]]:3: note: exit function called here
  (void)at_quick_exit(call_call_exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-44]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-59]]:3: note: exit function called here
};

// Non-compliant solutions calling quick_exit

void call_quick_exit() {
  quick_exit(0);
}

void call_call_quick_exit() {
  call_quick_exit();
}

extern int unknown_quick_exit_flag;

void call_quick_exit_conditionally() {
  if (unknown_quick_exit_flag)
    call_quick_exit();
}

void call_call_quick_exit_conditionally() {
  call_quick_exit_conditionally();
}

void test_quick_exit_called_directly() {
  (void)atexit(call_quick_exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-22]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-22]]:3: note: exit function called here
  (void)at_quick_exit(call_quick_exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-26]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-26]]:3: note: exit function called here
};

void test_quick_exit_called_indirectly() {
  (void)atexit(call_call_quick_exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-29]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-33]]:3: note: exit function called here
  (void)at_quick_exit(call_call_quick_exit);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-33]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-37]]:3: note: exit function called here
};

void test_conditional_quick_exit_called_directly() {
  (void)atexit(call_quick_exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-34]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-44]]:3: note: exit function called here
  (void)at_quick_exit(call_quick_exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-38]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-48]]:3: note: exit function called here
};

void test_conditional_quick_exit_called_indirectly() {
  (void)atexit(call_call_quick_exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-40]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-55]]:3: note: exit function called here
  (void)at_quick_exit(call_call_quick_exit_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-44]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-59]]:3: note: exit function called here
};

// Non-compliant solutions calling abort

void call_abort() {
  abort();
}

void call_call_abort() {
  call_abort();
}

extern int unknown_abort_flag;

void call_abort_conditionally() {
  if (unknown_abort_flag)
    call_abort();
}

void call_call_abort_conditionally() {
  call_abort_conditionally();
}

void test_abort_called_directly() {
  (void)atexit(call_abort);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-22]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-22]]:3: note: exit function called here
  (void)at_quick_exit(call_abort);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-26]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-26]]:3: note: exit function called here
};

void test_abort_called_indirectly() {
  (void)atexit(call_call_abort);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-29]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-33]]:3: note: exit function called here
  (void)at_quick_exit(call_call_abort);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-33]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-37]]:3: note: exit function called here
};

void test_conditional_abort_called_directly() {
  (void)atexit(call_abort_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-34]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-44]]:3: note: exit function called here
  (void)at_quick_exit(call_abort_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-38]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-48]]:3: note: exit function called here
};

void test_conditional_abort_called_indirectly() {
  (void)atexit(call_call_abort_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-40]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-55]]:3: note: exit function called here
  (void)at_quick_exit(call_call_abort_conditionally);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-44]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-59]]:3: note: exit function called here
};

// Mixed compliant and non-compliant solutions.

void call_exit2() {
  exit(0);
}

void test_compliant_and_noncompliant_atexits() {
  (void)atexit(cleanup1);
  (void)atexit(call_exit2);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-8]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-8]]:3: note: exit function called here
  (void)at_quick_exit(call_exit2);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-12]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-12]]:3: note: exit function called here
}

// Non-compliant solution using recursion.

extern int unknown_recursion_flag;

void recursive_hander() {
  if (unknown_recursion_flag > 0) {
    --unknown_recursion_flag;
    recursive_hander();
  }
  exit(0);
}

void test_recursive_handler() {
  (void)atexit(recursive_hander);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-11]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-7]]:3: note: exit function called here
  (void)at_quick_exit(recursive_hander);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-15]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-11]]:3: note: exit function called here
}

// Non-compliant solution using jumps.

jmp_buf env;
extern int unknown_error_flag;

void longjmp_handler() {
  if (setjmp(env)) {
    // error handling
  }

  if (unknown_error_flag) {
    longjmp(env, 255);
  }
}

void test_longjmp_handler() {
  (void)atexit(longjmp_handler);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls a longjmp instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-13]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-8]]:5: note: jump function called here
  (void)at_quick_exit(longjmp_handler);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls a longjmp instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-17]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-12]]:5: note: jump function called here
}

#if defined(CPPMODE) && defined(TEST_NS_NAME)
}
#endif

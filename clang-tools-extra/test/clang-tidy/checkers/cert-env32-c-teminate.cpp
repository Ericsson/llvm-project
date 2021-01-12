// Test that terminate is only detected in std namespace.
// RUN: %check_clang_tidy %s cert-env32-c %t

// Register handlers to run when exit is called.
int atexit(void (*)(void));

namespace std {
void terminate();

void callTerminate() {
  terminate();
}

void callCallTerminate() {
  callTerminate();
}

void testStdTerminate() {
  (void)atexit(callTerminate);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-11]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-11]]:3: note: exit function called here
  (void)atexit(callCallTerminate);
  // CHECK-NOTES: :[[@LINE-1]]:9: warning: exit-handler potentially calls an exit function instead of terminating normally with a return [cert-env32-c]
  // CHECK-NOTES: :[[@LINE-11]]:1: note: handler function declared here
  // CHECK-NOTES: :[[@LINE-15]]:3: note: exit function called here
}
} //namespace std

void terminate();

void callTerminate() {
  terminate();
}

void callCallTerminate() {
  callTerminate();
}

void testGlobalTerminate() {
  (void)atexit(callTerminate);
  (void)atexit(callCallTerminate);
}

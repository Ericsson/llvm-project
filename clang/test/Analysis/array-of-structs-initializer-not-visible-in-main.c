// RUN: %clang_analyze_cc1 -analyzer-checker=core,debug.ExprInspection -verify %s

// This test demonstrates that the analyzer cannot 'see' the initializer of a
// non-const array of structs during the analysis of the main() function.

void clang_analyzer_warnIfReached(void);

struct S {
  int a;
};

struct S struct_array[1] = {
  {11},
};

int num_array[1] = {22};

struct S single_struct = {33};

int main(int argc, char **argv) {
  if (argc == 1) {
    if (struct_array->a == 11) {
      clang_analyzer_warnIfReached(); // expected-warning {{REACHABLE}}
    } else {
      // FIXME: This should be unreachable, because the analyzer should realize
      // that when 'main' starts 'struct_array' still contains its initializer.
      clang_analyzer_warnIfReached(); // expected-warning {{REACHABLE}}
    }
  } else if (argc == 2) {
    if (*num_array == 22) {
      clang_analyzer_warnIfReached(); // expected-warning {{REACHABLE}}
    } else {
      clang_analyzer_warnIfReached(); // unreachable
    }
  } else {
    if (single_struct.a == 33) {
      clang_analyzer_warnIfReached(); // expected-warning {{REACHABLE}}
    } else {
      clang_analyzer_warnIfReached(); // unreachable
    }
  }
}

const struct S struct_array_const[1] = { {44} };

void use_struct_array(void) {
  if (struct_array->a == 44) {
    clang_analyzer_warnIfReached(); // expected-warning {{REACHABLE}}
  } else {
    clang_analyzer_warnIfReached(); // unreachable
  }
}

struct S struct_array_nonconst[1] = { {55} };

void use_struct_array_nonconst(void) {
  if (struct_array_nonconst->a == 55) {
    clang_analyzer_warnIfReached(); // expected-warning {{REACHABLE}}
  } else {
    // This is intentionally reachable, because this is a non-const array which
    // may have been changed before the call to this function.
    clang_analyzer_warnIfReached(); // expected-warning {{REACHABLE}}
  }
}

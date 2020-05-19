// RUN: %clang_analyze_cc1 -analyzer-opt-analyze-headers -analyzer-checker=security.FloatLoopCounter -analyzer-config report-in-system-header=false -isystem %S/system-include -verify %s

#include <system-header.h>

void normalFunction()
{
  for (float f = 0; f < 10; f += 1) // expected-warning {{Variable 'f' with floating point type 'float' should not be used as a loop counter}}
    ;

  systemFunction();
}

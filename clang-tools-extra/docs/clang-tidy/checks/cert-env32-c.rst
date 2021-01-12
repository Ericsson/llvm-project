.. title:: clang-tidy - cert-env32-c

cert-env32-c
============

Finds functions registered by ``atexit`` and ``at_quick_exit`` that are calling
exit functions ``_Exit``, ``exit``, ``quick_exit`` or ``longjmp``.

All exit handlers must return normally
--------------------------------------

The C Standard provides three functions that cause an application to terminate
normally: ``_Exit``, ``exit``, and ``quick_exit``. These are collectively called
exit functions. When the ``exit`` function is called, or control transfers out
of the ``main`` entry point function, functions registered with ``atexit`` are
called (but not ``at_quick_exit``). When the ``quick_exit`` function is called,
functions registered with ``at_quick_exit`` (but not ``atexit``) are called.
These functions are collectively called exit handlers. When the ``_Exit``
function is called, no exit handlers or signal handlers are called.

Exit handlers must terminate by returning. It is important and potentially
safety-critical for all exit handlers to be allowed to perform their cleanup
actions. This is particularly true because the application programmer does not
always know about handlers that may have been installed by support libraries.
Two specific issues include nested calls to an exit function and terminating a
call to an exit handler by invoking ``longjmp``.

A nested call to an exit function is undefined behavior. (See undefined behavior
182.) This behavior can occur only when an exit function is invoked from an exit
handler or when an exit function is called from within a signal handler.

If a call to the ``longjmp`` function is made that would terminate the call to a
function registered with ``atexit``, the behavior is undefined.

Noncompliant Code Example

In this noncompliant code example, the ``exit1`` and ``exit2`` functions are
registered by ``atexit`` to perform required cleanup upon program termination.
However, if ``some_condition`` evaluates to true, ``exit`` is called a second
time, resulting in undefined behavior.

.. code-block:: c

  #include <stdlib.h>
  
  void exit1(void) {
    /* ... Cleanup code ... */
    return;
  }
    
  void exit2(void) {
    extern int some_condition;
    if (some_condition) {
      /* ... More cleanup code ... */
      exit(0);
    }
    return;
  }
  
  int main(void) {
    if (atexit(exit1) != 0) {
      /* Handle error */
    }
    if (atexit(exit2) != 0) {
      /* Handle error */
    }
    /* ... Program code ... */
    return 0;
  }

Functions registered by the ``atexit`` function are called in the reverse order
from which they were registered. Consequently, if ``exit2`` exits in any way
other than by returning, ``exit1`` will not be executed. The same may also be
true for ``atexit`` handlers installed by support libraries.

Compliant Solution

A function that is registered as an exit handler by ``atexit`` must exit by
returning, as in this compliant solution:

.. code-block:: c

  #include <stdlib.h>
  
  void exit1(void) {
    /* ... Cleanup code ... */
    return;
  }
    
  void exit2(void) {
    extern int some_condition;
    if (some_condition) {
      /* ... More cleanup code ... */
    }
    return;
  }
  
  int main(void) {
    if (atexit(exit1) != 0) {
      /* Handle error */
    }
    if (atexit(exit2) != 0) {
      /* Handle error */
    }
    /* ... Program code ... */
    return 0;
  }

Noncompliant Code Example

In this noncompliant code example, ``exit1`` is registered by ``atexit`` so that
upon program termination, ``exit1`` is called. The ``exit1`` function jumps back
to ``main`` to return, with undefined results.

.. code-block:: c

#include <stdlib.h>
#include <setjmp.h>
 
jmp_buf env;
int val;
 
void exit1(void) {
  longjmp(env, 1);
}
 
int main(void) {
  if (atexit(exit1) != 0) {
    /* Handle error */
  }
  if (setjmp(env) == 0) {
    exit(0);
  } else {
    return 0;
  }
}

Compliant Solution

This compliant solution does not call ``longjmp`` but instead returns from the
exit handler normally:

.. code-block:: c

#include <stdlib.h>
 
void exit1(void) {
  return;
}
 
int main(void) {
  if (atexit(exit1) != 0) {
    /* Handle error */
  }
  return 0;
}

Description source: `<https://wiki.sei.cmu.edu/confluence/display/c/ENV32-C.+All+exit+handlers+must+return+normally>`_

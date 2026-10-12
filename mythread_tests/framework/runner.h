#ifndef RUNNER_H
#define RUNNER_H

typedef int (*test_fn_t)(void);

typedef struct {
    const char *name;
    test_fn_t   fn;
    int         timeout_ms;   /* 0 by default */
} test_case_t;

#define TEST_SUCCESS  0
#define TEST_FAILURE  1
#define TEST_TIMEOUT  2   /* set by the runner, not by tests */

/* Special-case marker for tests that terminate the process themselves
 * (e.g. mythread_exit from main): the runner treats exit code 42 as
 * success for such tests. */
#define TEST_EXIT_MARKER 42

int run_all(const test_case_t *tests, int n);

/* Emulates "return from main": ends the test child as if its main()
 * had returned `value` (a full exit(), so the process semantics with
 * still-running detached threads are exercised). Never returns.
 * Only meaningful inside a test run by the runner. */
void test_return_main(int value);

#endif

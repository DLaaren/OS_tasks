#include "mythread.h"
#include "assert.h"
#include "runner.h"   /* test_return_main */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>   /* exit */
#include <unistd.h>   /* usleep */

/* Cross-function lifecycle tests (L11..L15).
 *
 * The single-function suites (test_create/join/detach/exit.c) already
 * cover L1..L10 — every two-step path and both invalid orders. What
 * they cannot see is how the WHOLE process behaves around threads:
 * main exiting / returning while detached threads are still running,
 * and long-run resource hygiene (waves, handle reuse). These are the
 * areas where a 1.6 (kernel threads) and 1.7 (userspace pools) library
 * differ dramatically, so every test here names the exact contract it
 * enforces.
 *
 * L11 main_exit_detached   process semantics on mythread_exit
 * L12 main_return_detached process semantics on return from main
 * L13 join_chain           A creates B; main joins A; A joins B
 * L14 waves                repeated create/join bursts, no leak
 * L15 handle_reuse         recycled handle must behave as a fresh one
 */

/* Shared helpers ------------------------------------------------------ */

static void *
worker_noop(void *arg)
{
	(void)arg;
	return NULL;
}

static void *
worker_echo(void *arg)
{
	return arg;
}

struct gate {
	volatile int open;
};

static void *
worker_park(void *arg)
{
	struct gate *g = (struct gate *)arg;

	while (!__atomic_load_n(&g->open, __ATOMIC_SEQ_CST))
		; /* parked until released */

	return (void *)(intptr_t)0x11;
}

/*
 * Detached immortal: raises "up", then spins forever, heartbeating so
 * an observer can prove it is still alive. L11/L12 need threads that
 * outlive main.
 */
struct alive {
	volatile int up;
	volatile unsigned heartbeat;
};

static void *
worker_immortal(void *arg)
{
	struct alive *a = (struct alive *)arg;

	__atomic_store_n(&a->up, 1, __ATOMIC_SEQ_CST);
	while (1) {
		__atomic_fetch_add(&a->heartbeat, 1, __ATOMIC_SEQ_CST);
		for (volatile int i = 0; i < 1000; ++i)
			;
	}
	return NULL;   /* unreachable */
}

/* Tests --------------------------------------------------------------- */

/*
 * [L11] Main thread calls mythread_exit while detached threads are
 * still running. The course spec says mythread_exit from main ends
 * the process; POSIX says pthread_exit in main ends only the main
 * thread and the detached ones keep running. Both sane outcomes share
 * one hard requirement this test enforces: the transition must be
 * CLEAN — the process must not hang, must not crash, and mythread_exit
 * in main must not simply return.
 *
 * Choreography that yields a deterministic verdict in both worlds:
 * main spawns a watcher plus 4 detached immortals, publishes
 * "main is leaving", then mythread_exit's. The watcher (still running
 * after main is gone, exactly per POSIX semantics) proves the
 * immortals keep heartbeating and ends the process with the runner's
 * marker code 42. An implementation that instead kills the whole
 * process on main's exit also lands on a passing exit code (42 from
 * its own semantics, or 0) — course-spec-compliant. What FAILS here:
 * mythread_exit returning to the caller (exit 1), a hang (timeout),
 * a crash, or frozen detached threads (exit 7 from the watcher).
 */
struct l11ctx {
	struct alive *a;
	int n;
	volatile int main_gone;
};

static void *
worker_l11_watcher(void *arg)
{
	struct l11ctx *c = (struct l11ctx *)arg;

	while (!__atomic_load_n(&c->main_gone, __ATOMIC_SEQ_CST))
		;

	/* main has exited; per POSIX the detached threads must still be
	 * alive and running. Heartbeats must advance. */
	unsigned before[4];
	for (int i = 0; i < 4; ++i)
		before[i] = __atomic_load_n(&c->a[i].heartbeat,
		                            __ATOMIC_SEQ_CST);

	usleep(100 * 1000);

	for (int i = 0; i < 4; ++i) {
		unsigned now = __atomic_load_n(&c->a[i].heartbeat,
		                               __ATOMIC_SEQ_CST);
		if (now == before[i])
			exit(7);   /* detached thread froze after main exit */
	}

	/* process survived main's exit with detached threads running —
	 * report success through the runner's marker */
	exit(42);
}

int
test_main_exit_detached(void)
{
	static struct alive a[4];
	static mythread_t t[4];
	static struct l11ctx ctx;
	static mythread_t watcher;

	ctx.a = a;
	ctx.main_gone = 0;

	ASSERT_SUCCESS(mythread_create(&watcher, NULL, worker_l11_watcher,
	                               &ctx));

	for (int i = 0; i < 4; ++i) {
		a[i].up = 0;
		a[i].heartbeat = 0;
		ASSERT_SUCCESS(mythread_create(&t[i], NULL, worker_immortal,
		                               &a[i]));
		ASSERT_SUCCESS(mythread_detach(t[i]));
	}
	for (int i = 0; i < 4; ++i)
		while (!__atomic_load_n(&a[i].up, __ATOMIC_SEQ_CST))
			;

	__atomic_store_n(&ctx.main_gone, 1, __ATOMIC_SEQ_CST);
	mythread_exit((void *)(intptr_t)42);

	/* unreachable: either the watcher or the implementation itself
	 * ends the process; falling here means mythread_exit returned */
	return TEST_FAILURE;
}

/*
 * [L12] Main thread *returns* (implicitly exits) while a detached
 * thread is still running. Per C semantics: return from main IS
 * exit() — the whole process ends no matter how many threads are
 * alive. test_return_main() spells that out: a full exit(42) with the
 * runner's marker. The library-relevant part: process teardown with a
 * live detached thread must not hang or crash the library (the
 * classic 1.7 pool bug is teardown glue that deadlocks at exit).
 */
int
test_main_return_detached(void)
{
	static struct alive a;
	static mythread_t t;

	a.up = 0;
	a.heartbeat = 0;
	ASSERT_SUCCESS(mythread_create(&t, NULL, worker_immortal, &a));
	ASSERT_SUCCESS(mythread_detach(t));
	while (!__atomic_load_n(&a.up, __ATOMIC_SEQ_CST))
		;

	/* Emulated return from main: full process exit(42), detached
	 * immortal be damned — exactly what a return from main does. */
	test_return_main(42);
	return TEST_FAILURE;   /* unreachable */
}

/*
 * [L13] Nested chain: A creates B and joins it; main joins A and the
 * value from the chain's end travels through both joins. Already
 * probed one level (exit_nested), here the join is the *channel*: B
 * does not exit(), it returns.
 */
struct chain {
	mythread_t b;
};

static void *
worker_a(void *arg)
{
	struct chain *c = (struct chain *)arg;

	if (mythread_create(&c->b, NULL, worker_echo,
	                    (void *)(intptr_t)0xB0) != 0)
		return (void *)(intptr_t)-1;

	void *ret = NULL;
	if (mythread_join(c->b, &ret) != 0)
		return (void *)(intptr_t)-2;

	return ret;   /* pass B's result up to main */
}

int
test_join_chain(void)
{
	struct chain ch = { .b = 0 };
	mythread_t a;

	ASSERT_SUCCESS(mythread_create(&a, NULL, worker_a, &ch));

	void *ret = NULL;
	ASSERT_SUCCESS(mythread_join(a, &ret));
	ASSERT_EQ((intptr_t)ret, (intptr_t)0xB0);
	ASSERT_TRUE(ch.b != 0);
	return TEST_SUCCESS;
}

/*
 * [L14] Repeated create/join waves. Each wave creates W threads that
 * park, then releases and joins them all; WAVES rounds. Catches slow
 * leaks: a library that loses a slot/buffer per thread dies of
 * exhaustion mid-test (or hangs), while a healthy one survives every
 * wave and still works after.
 */
int
test_lifecycle_waves(void)
{
#define WAVES 25
#define WIDTH 8
	static struct gate g[WIDTH];
	static mythread_t t[WIDTH];

	for (int wave = 0; wave < WAVES; ++wave) {
		for (int i = 0; i < WIDTH; ++i) {
			g[i].open = 0;
			ASSERT_SUCCESS(mythread_create(&t[i], NULL,
			                               worker_park, &g[i]));
		}
		for (int i = 0; i < WIDTH; ++i)
			__atomic_store_n(&g[i].open, 1, __ATOMIC_SEQ_CST);
		for (int i = 0; i < WIDTH; ++i) {
			void *ret = NULL;
			ASSERT_SUCCESS(mythread_join(t[i], &ret));
			ASSERT_EQ((intptr_t)ret, (intptr_t)0x11);
		}
	}

	/* still fully functional after 200 thread lifecycles */
	mythread_t probe;
	ASSERT_SUCCESS(mythread_create(&probe, NULL, worker_noop, NULL));
	ASSERT_SUCCESS(mythread_join(probe, NULL));
#undef WAVES
#undef WIDTH
	return TEST_SUCCESS;
}

/*
 * [L15] Handle recycling: after a thread is reaped, its numeric handle
 * may legitimately be handed to a NEW thread. Two legal policies exist
 * (monotonic counter, or reuse) — we do not demand either. What must
 * hold in BOTH cases: after churn enough to tempt reuse, a fresh
 * create returns a handle that is cleanly joinable exactly once.
 * Joining it must not resurrect the old thread's state.
 */
int
test_handle_reuse(void)
{
#define BURN 64

	for (int i = 0; i < BURN; ++i) {
		mythread_t t;
		ASSERT_SUCCESS(mythread_create(&t, NULL, worker_noop, NULL));
		ASSERT_SUCCESS(mythread_join(t, NULL));
	}

	/* whatever policy the library has, a fresh thread must be a fresh,
	 * cleanly joinable thread */
	mythread_t t;
	ASSERT_SUCCESS(mythread_create(&t, NULL, worker_echo, NULL));

	void *ret = (void *)(intptr_t)0x51;   /* poisoned sentinel */
	ASSERT_SUCCESS(mythread_join(t, &ret));
	ASSERT_NULL(ret);
#undef BURN
	return TEST_SUCCESS;
}

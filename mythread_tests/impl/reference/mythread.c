/* Reference implementation: thin wrappers over POSIX pthreads.
 * Serves two purposes: sanity-checks the test framework itself, and
 * gives students a working example of the expected semantics. */

#include "mythread.h"
#include <errno.h>

int mythread_create(mythread_t *thread,
                    const mythread_attr_t *attr,
                    void *(*start_routine)(void *),
                    void *arg)
{
	return pthread_create((pthread_t *)thread, (pthread_attr_t *) attr, start_routine, arg);
}

int mythread_join(mythread_t thread, void **retval)
{
	return pthread_join((pthread_t)thread, retval);
}

int mythread_detach(mythread_t thread)
{
	return pthread_detach((pthread_t)thread);
}

void mythread_exit(void *retval)
{
	pthread_exit(retval);
}

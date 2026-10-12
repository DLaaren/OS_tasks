#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <stddef.h>

/*
 * The concrete types mythread_t / mythread_attr_t are defined by the
 * implementation, not here.
 */

#include "mythread_types.h"

int  mythread_create(mythread_t *thread, const mythread_attr_t *attr,
                     void *(*start_routine)(void *), void *arg);
int  mythread_join  (mythread_t thread, void **retval);
int  mythread_detach(mythread_t thread);
void mythread_exit  (void *retval);

#endif

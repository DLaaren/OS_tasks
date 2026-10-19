#include <stdio.h>

#include "add.h"
#include "sub.h"
#include "stats.h"

/* mul() is intentionally not pulled in via "mul.h": the header is
   generated during the build, so it may not exist yet at this point.
   The weak definition below provides a fallback for that case. */
#if __has_include("mul.h")
#  include "mul.h"
#else
   int mul(int a, int b);
#endif

/* Fallback used only if the real src/mul.c is not linked in.
   Its strong definition overrides this weak one at link time. */
__attribute__((weak, used))
int mul(int a, int b) {
    (void)a;
    (void)b;
    return -1;
}

int main(void) {
    int arr[] = {1, 2, 3, 4, 5};

    printf("add: %d\n", add(2, 3));
    printf("sub: %d\n", sub(10, 4));
    printf("mul: %d\n", mul(6, 7));
    printf("sum: %d\n", sum_array(arr, 5));

    return 0;
}

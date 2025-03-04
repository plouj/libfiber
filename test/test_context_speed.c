#include <fiber_context.h>
#include "test_helper.h"
#include <sys/time.h>

volatile int switchCounter = 0;

void* switch_to(void* param)
{
    fiber_context_t* ctx = (fiber_context_t*)param;
    while(1) {
        switchCounter += 1;
        fiber_swap_context(&ctx[1], &ctx[0]);
    }
    return NULL;
}


long long getusecs(struct timeval* tv)
{
    return (long long)tv->tv_sec * 1000000 + tv->tv_usec;
}

int main()
{
    /*
        this test creates a coroutine and switches to it.
        the coroutine simply switches back.
        we count the number of context switches and time it.
    */
    printf("testing fiber_context speed...\n");

    fiber_context_t ctx[2];

    test_assert(fiber_make_context_from_thread(&ctx[0]));
    test_assert(fiber_make_context(&ctx[1], 1024, &switch_to, ctx));

    struct timeval begin;
    gettimeofday(&begin, NULL);
    const int count = 10000000;
    while(switchCounter < count) {
        switchCounter += 1;
        fiber_swap_context(&ctx[0], &ctx[1]);
    }
    struct timeval end;
    gettimeofday(&end, NULL);

    test_assert(switchCounter == count);

    fiber_destroy_context(&ctx[1]);
    fiber_destroy_context(&ctx[0]);

    long long diff = getusecs(&end) - getusecs(&begin);
    printf("executed %d context switches in %lld usec (%lf seconds) = %lf switches per second\n", switchCounter, diff, (double)diff / 1000000.0, (double)switchCounter / ((double)diff / 1000000.0));

    printf("SUCCESS\n");    
    return 0;
}

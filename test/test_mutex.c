#include "fiber_mutex.h"
#include "fiber_manager.h"
#include "test_helper.h"

int volatile counter = 0;
fiber_mutex_t mutex;
#define PER_FIBER_COUNT 100000
#define NUM_FIBERS 100
#define NUM_THREADS 4

void* run_function(void* param)
{
    int i;
    for(i = 0; i < PER_FIBER_COUNT; ++i) {
        fiber_mutex_lock(&mutex);
        ++counter;
        fiber_mutex_unlock(&mutex);
    }
    return NULL;
}

int main()
{
    fiber_manager_set_total_kernel_threads(NUM_THREADS);

    fiber_mutex_init(&mutex);

    fiber_t* fibers[NUM_FIBERS];
    int i;
    for(i = 0; i < NUM_FIBERS; ++i) {
        fibers[i] = fiber_create(20000, &run_function, NULL);
    }

    for(i = 0; i < NUM_FIBERS; ++i) {
        fiber_join(fibers[i]);
    }

    test_assert(counter == NUM_FIBERS * PER_FIBER_COUNT);

    return 0;
}


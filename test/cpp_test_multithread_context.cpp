#include "fiber_manager.h"
#include "test_helper.h"
#include <pthread.h>
#include <list>

std::list<fiber_context_t*> contexts_to_run;
pthread_mutex_t context_mutex = PTHREAD_MUTEX_INITIALIZER;

void schedule(fiber_context_t* ctx)
{
    pthread_mutex_lock(&context_mutex);
    contexts_to_run.push_back(ctx);
    pthread_mutex_unlock(&context_mutex);
}

fiber_context_t* get_for_running()
{
    fiber_context_t* ret = NULL;
    pthread_mutex_lock(&context_mutex);
    if(!contexts_to_run.empty()) {
        ret = contexts_to_run.front();
        contexts_to_run.pop_front();
    }
    pthread_mutex_unlock(&context_mutex);
    return ret;
}

__thread int id = 0;
__thread int counter = 0;
__thread fiber_context_t* current_context = NULL;
__thread fiber_context_t* to_schedule_after_switch = NULL;

void yield()
{
    fiber_context_t* to_run = NULL;
    while(!(to_run = get_for_running())) { pthread_yield();}
    to_schedule_after_switch = current_context;
    current_context = to_run;
    fiber_swap_context(to_schedule_after_switch, to_run);
}

void* run_func(void* p)
{
    id = 1;
    fiber_context_t main_ctx;
    fiber_make_context_from_thread(&main_ctx);
    current_context = &main_ctx;
    int count = 0;
    while(1) {
        yield();
        if(to_schedule_after_switch) schedule(to_schedule_after_switch);
        ++count;
        printf("run_func %d in thread %d\n", count, id);
    }
    return NULL;
}

void test_func(void* p)
{
    if(to_schedule_after_switch) schedule(to_schedule_after_switch);
    while(1) {
        yield();
        if(to_schedule_after_switch) schedule(to_schedule_after_switch);
        ++counter;
        printf("id: %d counter: %d\n", id, counter);
    }
}

int main()
{
    printf("testing multithread contexts\n");
    fiber_context_t main_ctx;
    fiber_make_context_from_thread(&main_ctx);
    current_context = &main_ctx;
    for(int i = 0; i < 10; ++i) {
        fiber_context_t* ctx = (fiber_context_t*)malloc(sizeof(fiber_context_t));
        fiber_make_context(ctx, 100000, &test_func, NULL);
        schedule(ctx);
    }

    pthread_t reader;
    pthread_create(&reader, NULL, &run_func, NULL);

    int count = 0;
    while(1) {
        yield();
        if(to_schedule_after_switch) schedule(to_schedule_after_switch);
        ++count;
        printf("main %d in thread %d\n", count, id);
    }

    return 0;
}

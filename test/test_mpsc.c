#include <mpsc_fifo.h>
#include "test_helper.h"
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#define PUSH_COUNT 10000000
#define NUM_THREADS 4

mpsc_fifo_t fifo;
int results[PUSH_COUNT] = {};

void* push_func(void* p)
{
    intptr_t i;
    for(i = 0; i < PUSH_COUNT; ++i) {
        mpsc_node_t* const node = malloc(sizeof(mpsc_node_t));
        node->data = (void*)i;
        mpsc_fifo_push(&fifo, node);
    }
    return NULL;
}

int main()
{
    mpsc_fifo_init(&fifo);

    pthread_t producers[NUM_THREADS];
    intptr_t i = 0;
    for(i = 1; i < NUM_THREADS; ++i) {
        pthread_create(&producers[i], NULL, &push_func, NULL);
    }

    usleep(500000);//TODO: use pthread_barrier

    mpsc_node_t* node = NULL;
    for(i = 0; i < PUSH_COUNT * (NUM_THREADS-1); ++i) {
        while(!(node = mpsc_fifo_pop(&fifo))) {};
        ++results[(intptr_t)node->data];
        free(node);
    }

    for(i = 1; i < NUM_THREADS; ++i) {
        pthread_join(producers[i], 0);
    }

    for(i = 0; i < PUSH_COUNT; ++i) {
        test_assert(results[i] == (NUM_THREADS - 1));
    }

    printf("cleaning...\n");
    mpsc_fifo_destroy(&fifo);

    return 0;
}

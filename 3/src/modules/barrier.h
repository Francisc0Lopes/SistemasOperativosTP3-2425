#ifndef BARRIER_H
#define BARRIER_H 

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int num_threads;
    int phase;
} sot_barrier_t;

int sot_barrier_init (sot_barrier_t *barrier, int numberOfThreads);
int sot_barrier_destroy (sot_barrier_t *barrier);
int sot_barrier_wait (sot_barrier_t *barrier); 


#endif // BARRIER_H

#include "barrier.h"




int sot_barrier_init (sot_barrier_t *barrier, int numberOfThreads){
    barrier->count = 0;
    barrier->num_threads = numberOfThreads;
    barrier->phase = 0;
    pthread_mutex_init(&barrier->mutex, NULL);
    pthread_cond_init(&barrier->cond, NULL);
    return 0;
}

int sot_barrier_destroy (sot_barrier_t *barrier){
    pthread_mutex_destroy(&barrier->mutex);
    pthread_cond_destroy(&barrier->cond);
    return 0;
}

int sot_barrier_wait (sot_barrier_t *barrier){
    pthread_mutex_lock(&barrier->mutex);
    int local_phase = barrier->phase;
    barrier->count++;
    if(barrier->count == barrier->num_threads){
        barrier->count = 0;
        barrier->phase++;
        pthread_cond_broadcast(&barrier->cond);
    } else {
        while(local_phase == barrier->phase){
            pthread_cond_wait(&barrier->cond, &barrier->mutex);
        }
    }
    pthread_mutex_unlock(&barrier->mutex);
    return 0;
}
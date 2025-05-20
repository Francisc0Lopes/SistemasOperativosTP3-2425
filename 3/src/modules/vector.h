#ifndef VECTOR_H
#define VECTOR_H
#include "barrier.h"
typedef struct {
    int *v;
    size_t v_sz;
    int nThreads;
    int tid;
    int *min_local;
    int *max_local;
    int *sum_local;
    double *mean;
    sot_barrier_t *barrier;
    int *min_global;
    int *max_global;
} thread_arg_t;

void norm_min_max_and_classify_parallel(int v[], size_t v_sz, int nThreads); 
void *worker(void *arg);


#endif // VECTOR_H
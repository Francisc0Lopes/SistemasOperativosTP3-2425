

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include "vector.h"
#include "barrier.h"



void *worker(void *arg) {
    thread_arg_t *a = (thread_arg_t*)arg;
    size_t chunk = a->v_sz / a->nThreads;
    size_t start = a->tid * chunk;
    size_t end = (a->tid == a->nThreads-1) ? a->v_sz : start + chunk;

    int min = a->v[start], max = a->v[start];
    for(size_t i = start+1; i < end; ++i){
        if (a->v[i] < min) min = a->v[i];
        if (a->v[i] > max) max = a->v[i];
    }
    a->min_local[a->tid] = min;
    a->max_local[a->tid] = max;

    sot_barrier_wait(a->barrier);

    if(a->tid == 0){
        int min_g = a->min_local[0], max_g = a->max_local[0];
        for(int i = 1; i < a->nThreads; ++i){
            if (a->min_local[i] < min_g) min_g = a->min_local[i];
            if (a->max_local[i] > max_g) max_g = a->max_local[i];
        }
        *(a->min_global) = min_g;
        *(a->max_global) = max_g;
    }

    sot_barrier_wait(a->barrier);

    int min_g = *(a->min_global);
    int max_g = *(a->max_global);

    int sum = 0;
    for(size_t i = start; i < end; ++i){
        if(max_g == min_g){
            a->v[i] = 0;
        }else{
            a->v[i] = (int)round((a->v[i] - min_g) * 100.0 / (max_g - min_g));
        }
        sum += a->v[i];
    }
    a->sum_local[a->tid] = sum;

    sot_barrier_wait(a->barrier);

    if(a->tid == 0){
        int total = 0;
        for (int i = 0; i < a->nThreads; ++i) total += a->sum_local[i];
        *(a->mean) = (double)total / a->v_sz;
    }

    sot_barrier_wait(a->barrier);

    double mean = *(a->mean);

    for(size_t i = start; i < end; ++i){
        a->v[i] = (a->v[i] >= mean) ? 1 : 0;
    }

    return NULL;
}

void norm_min_max_and_classify_parallel(int v[], size_t v_sz, int nThreads){
    pthread_t *threads = malloc(nThreads * sizeof(pthread_t));
    thread_arg_t *args = malloc(nThreads * sizeof(thread_arg_t));
    int *min_local = malloc(nThreads * sizeof(int));
    int *max_local = malloc(nThreads * sizeof(int));
    int *sum_local = malloc(nThreads * sizeof(int));
    int min_global = 0, max_global = 0;
    double mean = 0.0;
    sot_barrier_t barrier;
    sot_barrier_init(&barrier, nThreads);

    for(int i = 0; i < nThreads; ++i){
        args[i].v = v;
        args[i].v_sz = v_sz;
        args[i].nThreads = nThreads;
        args[i].tid = i;
        args[i].min_local = min_local;
        args[i].max_local = max_local;
        args[i].sum_local = sum_local;
        args[i].mean = &mean;
        args[i].barrier = &barrier;
        args[i].min_global = &min_global;
        args[i].max_global = &max_global;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }
    for(int i = 0; i < nThreads; ++i){
        pthread_join(threads[i], NULL);
    }
    sot_barrier_destroy(&barrier);
    free(threads);
    free(args);
    free(min_local);
    free(max_local);
    free(sum_local);
}
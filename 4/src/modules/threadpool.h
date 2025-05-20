#ifndef THREADPOOL_H
#define THREADPOOL_H

typedef void *(*wi_function_t)(void *);

typedef struct threadpool threadpool_t;

int threadpool_init(threadpool_t **tp, int queueDim, int nthreads_min, int nthreads_max);
int threadpool_submit(threadpool_t *tp, wi_function_t func, void *args);
int threadpool_destroy(threadpool_t *tp);

#endif
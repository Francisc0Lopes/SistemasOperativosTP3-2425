#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include "threadpool.h"
#include "sharedbuffer.h"

typedef struct {
    wi_function_t func;
    void *args;
} work_item_t;

struct threadpool {
    int nthreads_min, nthreads_max;
    pthread_t *threads;
    int nthreads;
    sharedbuffer_t *queue;
    bool accepting;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

static void *worker_thread(void *arg) {
    threadpool_t *tp = (threadpool_t *)arg;
    while (1) {
        work_item_t *item = sharedbuffer_get(tp->queue);
        if (item == NULL) continue;
        if (item->func == NULL) { // sinal de terminação
            free(item);
            break;
        }
        item->func(item->args);
        free(item);
    }
    return NULL;
}

int threadpool_init(threadpool_t **tp_ptr, int queueDim, int nthreads_min, int nthreads_max) {
    threadpool_t *tp = malloc(sizeof(threadpool_t));
    tp->nthreads_min = nthreads_min;
    tp->nthreads_max = nthreads_max;
    tp->nthreads = nthreads_min;
    tp->threads = malloc(sizeof(pthread_t) * nthreads_max);
    tp->queue = sharedbuffer_create(queueDim);
    tp->accepting = true;
    pthread_mutex_init(&tp->lock, NULL);
    pthread_cond_init(&tp->cond, NULL);
    for (int i = 0; i < nthreads_min; ++i)
        pthread_create(&tp->threads[i], NULL, worker_thread, tp);
    *tp_ptr = tp;
    return 0;
}

int threadpool_submit(threadpool_t *tp, wi_function_t func, void *args) {
    pthread_mutex_lock(&tp->lock);
    if (!tp->accepting) {
        pthread_mutex_unlock(&tp->lock);
        return -1;
    }
    work_item_t *item = malloc(sizeof(work_item_t));
    item->func = func;
    item->args = args;
    sharedbuffer_put(tp->queue, item);
    pthread_mutex_unlock(&tp->lock);
    return 0;
}

int threadpool_destroy(threadpool_t *tp) {
    pthread_mutex_lock(&tp->lock);
    tp->accepting = false;
    pthread_mutex_unlock(&tp->lock);

    // Envia sinais de terminação para cada worker
    for (int i = 0; i < tp->nthreads; ++i) {
        work_item_t *item = malloc(sizeof(work_item_t));
        item->func = NULL;
        item->args = NULL;
        sharedbuffer_put(tp->queue, item);
    }
    for (int i = 0; i < tp->nthreads; ++i)
        pthread_join(tp->threads[i], NULL);

    sharedbuffer_destroy(tp->queue);
    free(tp->threads);
    pthread_mutex_destroy(&tp->lock);
    pthread_cond_destroy(&tp->cond);
    free(tp);
    return 0;
}
#include <stdlib.h>
#include <pthread.h>
#include "sharedbuffer.h"

struct sharedbuffer {
    void **buffer;
    int capacity;
    int count;
    int in;
    int out;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

sharedbuffer_t *sharedbuffer_create(int capacity){
    sharedbuffer_t *sb = malloc(sizeof(sharedbuffer_t));
    sb->buffer = malloc(sizeof(void *) * capacity);
    sb->capacity = capacity;
    sb->count = 0;
    sb->in = 0;
    sb->out = 0;
    pthread_mutex_init(&sb->mutex, NULL);
    pthread_cond_init(&sb->not_empty, NULL);
    pthread_cond_init(&sb->not_full, NULL);
    return sb;
}

void sharedbuffer_destroy(sharedbuffer_t *sb){
    pthread_mutex_destroy(&sb->mutex);
    pthread_cond_destroy(&sb->not_empty);
    pthread_cond_destroy(&sb->not_full);
    free(sb->buffer);
    free(sb);
}

void sharedbuffer_put(sharedbuffer_t *sb, void *item){
    pthread_mutex_lock(&sb->mutex);
    while(sb->count == sb->capacity){
        pthread_cond_wait(&sb->not_full, &sb->mutex);
    }
    sb->buffer[sb->in] = item;
    sb->in = (sb->in + 1) % sb->capacity;
    sb->count++;
    pthread_cond_signal(&sb->not_empty);
    pthread_mutex_unlock(&sb->mutex);
}

void *sharedbuffer_get(sharedbuffer_t *sb){
    pthread_mutex_lock(&sb->mutex);
    while(sb->count == 0){
        pthread_cond_wait(&sb->not_empty, &sb->mutex);
    }
    void *item = sb->buffer[sb->out];
    sb->out = (sb->out + 1) % sb->capacity;
    sb->count--;
    pthread_cond_signal(&sb->not_full);
    pthread_mutex_unlock(&sb->mutex);
    return item;
}
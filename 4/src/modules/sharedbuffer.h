#ifndef SHAREDBUFFER_H
#define SHAREDBUFFER_H

typedef struct sharedbuffer sharedbuffer_t;

sharedbuffer_t *sharedbuffer_create(int capacity);
void sharedbuffer_destroy(sharedbuffer_t *sb);
void sharedbuffer_put(sharedbuffer_t *sb, void *item);
void *sharedbuffer_get(sharedbuffer_t *sb);

#endif
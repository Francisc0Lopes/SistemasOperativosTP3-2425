#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "modules/threadpool.h"

void *test_func(void *arg) {
    int id = *(int *)arg;
    printf("Trabalho %d executado pelo thread pool\n", id);
    free(arg);
    sleep(1);
    return NULL;
}

int main() {
    threadpool_t *tp;
    threadpool_init(&tp, 10, 2, 4);

    for(int i = 0; i < 8; ++i){
        int *id = malloc(sizeof(int));
        *id = i;
        threadpool_submit(tp, test_func, id);
    }

    threadpool_destroy(tp);
    printf("Thread pool terminado.\n");
    return 0;
}
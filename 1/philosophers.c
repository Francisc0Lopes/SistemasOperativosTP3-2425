#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5
#define NUM_MEALS 3

pthread_mutex_t forks[NUM_PHILOSOPHERS];

void think(int id) {
    printf("Filósofo %d está a pensar.\n", id);
    sleep(rand() % 2 + 1);
}

void eat(int id) {
    printf("Filósofo %d está a comer.\n", id);
    sleep(rand() % 2 + 1);
}

void* philosopher(void* arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;

    for (int i = 0; i < NUM_MEALS; ++i) {
        think(id);

        if (id % 2 == 0) {
            pthread_mutex_lock(&forks[left]);
            pthread_mutex_lock(&forks[right]);
        } else {
            pthread_mutex_lock(&forks[right]);
            pthread_mutex_lock(&forks[left]);
        }

        eat(id);

        pthread_mutex_unlock(&forks[left]);
        pthread_mutex_unlock(&forks[right]);
    }

    printf("Filósofo %d terminou.\n", id);
    free(arg);
    return NULL;
}

int main() {
    pthread_t threads[NUM_PHILOSOPHERS];

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        if (pthread_mutex_init(&forks[i], NULL) != 0) {
            perror("Erro a inicializar mutex");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        int* id = malloc(sizeof(int));
        if (id == NULL) {
            perror("Erro a alocar memória");
            exit(EXIT_FAILURE);
        }
        *id = i;
        if (pthread_create(&threads[i], NULL, philosopher, id) != 0) {
            perror("Erro a criar thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        pthread_mutex_destroy(&forks[i]);
    }

    return 0;
}

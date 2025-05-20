#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "modules/accounts.h"
#include "modules/transfer.h"

#define ACCOUNTS_MAX 10
#define ACCOUNT_BALANCE 1000.0
#define N_THREADS 5

typedef struct {
    account_t *accounts;
    pthread_mutex_t *locks;
    int num_accounts;
} simulation_t;

int main() {
    srand(time(NULL));
    account_t accounts[ACCOUNTS_MAX];
    pthread_mutex_t locks[ACCOUNTS_MAX];

    // Inicializa contas e mutexes
    initialize_accounts(accounts, ACCOUNTS_MAX, ACCOUNT_BALANCE);
    for (int i = 0; i < ACCOUNTS_MAX; i++) {
        pthread_mutex_init(&locks[i], NULL);
    }

    pthread_t threads[N_THREADS];
    simulation_t sim = {accounts, locks, ACCOUNTS_MAX};

    for (int i = 0; i < N_THREADS; i++) {
        pthread_create(&threads[i], NULL, perform_transfers, (void*)&sim);
    }

    for (int i = 0; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    double total_balance = 0.0;
    for (int i = 0; i < ACCOUNTS_MAX; i++) {
        total_balance += accounts[i].balance;
    }

    printf("Total balance after transfers: %.2f\n", total_balance);

    // Destroi os mutexes
    for (int i = 0; i < ACCOUNTS_MAX; i++) {
        pthread_mutex_destroy(&locks[i]);
    }

    return 0;
}
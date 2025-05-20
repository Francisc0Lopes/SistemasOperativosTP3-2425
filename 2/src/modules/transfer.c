#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "accounts.h"
#include "transfer.h"

#define NUM_TRANSFERS_PER_THREAD 1000

// Adiciona um mutex para cada conta
typedef struct {
    account_t *accounts;
    pthread_mutex_t *locks;
    int num_accounts;
} simulation_t;

void transfer(account_t *accounts, pthread_mutex_t *locks, int from, int to, double amount) {
    // Ordena os índices para evitar deadlock
    int first = from < to ? from : to;
    int second = from < to ? to : from;

    pthread_mutex_lock(&locks[first]);
    pthread_mutex_lock(&locks[second]);

    if (accounts[from].balance >= amount) {
        accounts[from].balance -= amount;
        accounts[to].balance += amount;
    }

    pthread_mutex_unlock(&locks[second]);
    pthread_mutex_unlock(&locks[first]);
}

void *perform_transfers(void *arg) {
    simulation_t *sim = (simulation_t *)arg;
    for (int i = 0; i < NUM_TRANSFERS_PER_THREAD; i++) {
        int from = rand() % sim->num_accounts;
        int to = rand() % sim->num_accounts;
        while (to == from) {
            to = rand() % sim->num_accounts;
        }
        double amount = (rand() % 100) + 1;
        transfer(sim->accounts, sim->locks, from, to, amount);
    }
    return NULL;
}
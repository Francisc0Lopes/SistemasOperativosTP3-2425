#ifndef TRANSFER_H
#define TRANSFER_H

#include "accounts.h"

void *perform_transfers(void *arg);
void transfer(account_t *accounts, pthread_mutex_t *locks, int from, int to, double amount);

#endif // TRANSFER_H
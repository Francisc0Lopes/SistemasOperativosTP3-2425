#include <stdio.h>
#include <stdlib.h>
#include "accounts.h"

void initialize_accounts(account_t *accounts, int num_accounts, double initial_balance) {
    for(int i = 0; i < num_accounts; i++){
        accounts[i].id = i;
        accounts[i].balance = initial_balance;
    }
}

double get_balance(account_t *accounts, int account_id) {
    return accounts[account_id].balance;
}

void deposit(account_t *accounts, int account_id, double amount) {
    accounts[account_id].balance += amount;
}

int withdraw(account_t *accounts, int account_id, double amount) {
    if(accounts[account_id].balance >= amount){
        accounts[account_id].balance -= amount;
        return 1; // sucesso
    }
    return 0; // saldo insuficiente
}
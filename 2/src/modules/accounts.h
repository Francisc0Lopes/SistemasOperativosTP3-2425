#ifndef ACCOUNTS_H
#define ACCOUNTS_H

typedef struct {
    int id;
    double balance;
} account_t;

void initialize_accounts(account_t *accounts, int num_accounts, double initial_balance); 
double get_balance(account_t *accounts, int account_id);
void deposit(account_t *accounts, int account_id, double amount); 
int withdraw(account_t *accounts, int account_id, double amount);

#endif // ACCOUNTS_H
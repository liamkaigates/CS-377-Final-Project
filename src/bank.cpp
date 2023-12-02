#include <bank.h>

/**
 * @brief prints account information
 */
void Bank::print_account()
{
  for (int i = 0; i < num; i++)
  {
    accounts[i].lock_read();
    cout << "ID# " << accounts[i].accountID << " | " << accounts[i].balance
         << endl;
    accounts[i].unlock_read();
  }

  pthread_mutex_lock(&bank_lock);
  cout << "Success: " << num_succ << " Fails: " << num_fail << endl;
  pthread_mutex_unlock(&bank_lock);
}

/**
 * @brief helper function to increment the bank variable `num_fail` and log
 *        message.
 *
 * @param message
 */
void Bank::recordFail(char *message)
{
  pthread_mutex_lock(&bank_lock);
  cout << message << endl;
  num_fail++;
  pthread_mutex_unlock(&bank_lock);
}

/**
 * @brief helper function to increment the bank variable `num_succ` and log
 *        message.
 *
 * @param message
 */
void Bank::recordSucc(char *message)
{
  pthread_mutex_lock(&bank_lock);
  cout << message << endl;
  num_succ++;
  pthread_mutex_unlock(&bank_lock);
}

/**
 * @brief Construct a new Bank:: Bank object.
 *
 * Requirements:
 *  - The function should initialize the private variables.
 *  - Create a new array[N] of type Accounts.
 *  - Initialize each account (HINT: there are three fields to initialize)
 *
 * @param N
 */
Bank::Bank(int N)
{
  pthread_mutex_init(&bank_lock, NULL);
  num = N;
  num_succ = 0;
  num_fail = 0;
  accounts = (Account *)malloc(N * sizeof(Account));
  for (int i = 0; i < N; i++)
  {
    accounts[i].accountID = i;
    accounts[i].balance = 0;
    accounts[i].read_count = 0;
    accounts[i].write_lock = PTHREAD_MUTEX_INITIALIZER;
    accounts[i].read_lock = PTHREAD_MUTEX_INITIALIZER;
  }
}

/**
 * @brief Destroy the Bank:: Bank object
 *
 * Requirements:
 *  - Make sure to destroy all locks.
 *  - Make sure to free all memory
 *
 */
Bank::~Bank()
{
  for (int i = 0; i < num; i++)
  {
    pthread_mutex_destroy(&accounts[i].read_lock);
    pthread_mutex_destroy(&accounts[i].write_lock);
  }
  free(accounts);
  pthread_mutex_destroy(&bank_lock);
}

/**
 * @brief Adds money to an account
 *
 * Requirements:
 *  - Make sure to log in the following format
 *    `Worker [worker_id] completed ledger [ledger_id]: deposit [amount] into account [account]`
 *
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param accountID the account ID to deposit
 * @param amount the amount deposited
 * @return int
 */
int Bank::deposit(int workerID, int ledgerID, int accountID, int amount)
{
  if (amount >= 0)
  {
    accounts[accountID].lock_write();
    accounts[accountID].balance += amount;
    string str = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": deposit " + to_string(amount) + " into account " + to_string(accountID);
    char message[str.length() + 1];
    message[str.length()] = '\0';
    for (int i = 0; i < str.length(); i++)
    {
      message[i] = str[i];
    }
    recordSucc(message);
    accounts[accountID].unlock_write();
    return 0;
  }
}

/**
 * @brief Withdraws money from an account
 *
 * Requirements:
 *  - Make sure the account has a large enough balance.
 *    - Case 1: withdraw amount <= balance, log success
 *    - Case 2: log failure
 *
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param accountID the account ID to withdraw
 * @param amount the amount withdrawn
 * @return int 0 on success -1 on failure
 */
int Bank::withdraw(int workerID, int ledgerID, int accountID, int amount)
{
  accounts[accountID].lock_write();
  if (accounts[accountID].balance > amount && amount >= 0)
  {
    accounts[accountID].balance -= amount;
    string str = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": withdraw " + to_string(amount) + " from account " + to_string(accountID);
    char message[str.length() + 1];
    message[str.length()] = '\0';
    for (int i = 0; i < str.length(); i++)
    {
      message[i] = str[i];
    }
    recordSucc(message);
    accounts[accountID].unlock_write();
  }
  else
  {
    string str = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": withdraw " + to_string(amount) + " from account " + to_string(accountID);
    char message[str.length() + 1];
    message[str.length()] = '\0';
    for (int i = 0; i < str.length(); i++)
    {
      message[i] = str[i];
    }
    recordFail(message);
    accounts[accountID].unlock_write();
    return -1;
  }
  return 0;
}

/**
 * @brief Transfer from one account to another
 *
 * Requirements:
 *  - Make sure there is enough money in the FROM account
 *  - Be careful with the locking order
 *
 *
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param srcID the account to transfer money out
 * @param destID the account to receive the money
 * @param amount the amount to transfer
 * @return int 0 on success -1 on error
 */
int Bank::transfer(int workerID, int ledgerID, int srcID, int destID,
                   unsigned int amount)
{
  accounts[srcID].lock_write();
  if (accounts[srcID].balance > amount && srcID != destID && amount >= 0)
  {
    accounts[srcID].balance -= amount;
    accounts[destID].lock_write();
    accounts[destID].balance += amount;
    accounts[destID].unlock_write();
    string str = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": transfer " + to_string(amount) + " from account " + to_string(srcID) + " to account " + to_string(destID);
    char message[str.length() + 1];
    message[str.length()] = '\0';
    for (int i = 0; i < str.length(); i++)
    {
      message[i] = str[i];
    }
    recordSucc(message);
    accounts[srcID].unlock_write();
  }
  else
  {
    string str = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": transfer " + to_string(amount) + " from account " + to_string(srcID) + " to account " + to_string(destID);
    char message[str.length() + 1];
    message[str.length()] = '\0';
    for (int i = 0; i < str.length(); i++)
    {
      message[i] = str[i];
    }
    recordFail(message);
    accounts[srcID].unlock_write();
    return -1;
  }
  return 0;
}

/**
 * @brief Prints the balance of an account
 *
 * Requirements:
 * - Log the success or failure
 *
 *
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param accountID the account ID to print balance of
 * @return int 0 on success -1 on error
 */
int Bank::check_balance(int workerID, int ledgerID, int accountID)
{
  accounts[accountID].lock_read();
  int balance = accounts[accountID].balance;
  pthread_mutex_lock(&bank_lock);
  cout << "Account " << accountID << " - Balance: " << balance << endl;
  pthread_mutex_unlock(&bank_lock);
  string str = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": check balance of account " + to_string(accountID);
  char message[str.length() + 1];
  message[str.length()] = '\0';
  for (int i = 0; i < str.length(); i++)
  {
    message[i] = str[i];
  }
  recordSucc(message);
  accounts[accountID].unlock_read();
  return 0;
}

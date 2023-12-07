#include <bank.h>

/**
 * @brief prints account information
 */
void Bank::print_account()
{
  for (int i = 0; i < num; i++)
  {
    accounts[i].lock_read(); // lock read
    cout << "ID# " << accounts[i].accountID << " | " << accounts[i].balance
         << endl;              // print account info
    accounts[i].unlock_read(); // unlock read
  }

  pthread_mutex_lock(&bank_lock);                                    // lock bank
  cout << "Success: " << num_succ << " Fails: " << num_fail << endl; // print success and fails
  pthread_mutex_unlock(&bank_lock);                                  // unlock bank
}

/**
 * @brief helper function to increment the bank variable `num_fail` and log
 *        message.
 *
 * @param message
 */
void Bank::recordFail(char *message)
{
  pthread_mutex_lock(&bank_lock);   // lock bank
  cout << message << endl;          // print message
  num_fail++;                       // increment fails
  pthread_mutex_unlock(&bank_lock); // unlock bank
}

/**
 * @brief helper function to increment the bank variable `num_succ` and log
 *        message.
 *
 * @param message
 */
void Bank::recordSucc(char *message)
{
  pthread_mutex_lock(&bank_lock);   // lock bank
  cout << message << endl;          // print message
  num_succ++;                       // increment success
  pthread_mutex_unlock(&bank_lock); // unlock bank
}

/**
 * @brief Construct a new Bank:: Bank object.
 *
 * Requirements:
 *  - The function should initialize the private variables.
 *  - Create a new array[N] of type Accounts.
 *  - Initialize each account (HINT: there are three fields to initialize)
 *  - Create a new array[N] of type AccountLog.
 *  - Initialize each account log (HINT: there are two fields to initialize)
 *
 * @param N
 */
Bank::Bank(int N)
{
  pthread_mutex_init(&bank_lock, NULL);              // initialize bank lock
  num = N;                                           // set num to N
  num_succ = 0;                                      // set num_succ to 0
  num_fail = 0;                                      // set num_fail to 0
  accounts = (Account *)malloc(N * sizeof(Account)); // allocate memory for accounts
  for (int i = 0; i < N; i++)
  {
    accounts[i].accountID = i;                          // set accountID to i
    accounts[i].balance = 0;                            // set balance to 0
    accounts[i].read_count = 0;                         // set read_count to 0
    accounts[i].write_lock = PTHREAD_MUTEX_INITIALIZER; // initialize write_lock
    accounts[i].read_lock = PTHREAD_MUTEX_INITIALIZER;  // initialize read_lock
  }
  accountLogs = (AccountLog *)malloc(N * sizeof(AccountLog)); // allocate memory for accountLogs
  for (int i = 0; i < N; i++)
  {
    accountLogs[i].write_lock = PTHREAD_MUTEX_INITIALIZER; // initialize write_lock
    accountLogs[i].read_lock = PTHREAD_MUTEX_INITIALIZER;  // initialize read_lock
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
    pthread_mutex_destroy(&accounts[i].read_lock);  // destroy read_lock
    pthread_mutex_destroy(&accounts[i].write_lock); // destroy write_lock
  }
  for (int i = 0; i < num; i++)
  {
    pthread_mutex_destroy(&accountLogs[i].read_lock);  // destroy read_lock
    pthread_mutex_destroy(&accountLogs[i].write_lock); // destroy write_lock
  }
  free(accounts);                    // free memory
  free(accountLogs);                 // free memory
  pthread_mutex_destroy(&bank_lock); // destroy bank_lock
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
 * @param file the file to write the log to
 * @return int
 */
int Bank::deposit(int workerID, int ledgerID, int accountID, int amount, fstream *file)
{
  if (amount >= 0)
  {
    accounts[accountID].lock_write();                                                                                                                                       // lock account
    accounts[accountID].balance += amount;                                                                                                                                  // add amount to balance
    string str = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": deposit " + to_string(amount) + " into account " + to_string(accountID); // create log message
    string log = "Transaction Type: Deposit, Amount: " + to_string(amount) + ", Status: Success\n";                                                                         // create log message
    char message[str.length() + 1];                                                                                                                                         // create char array
    message[str.length()] = '\0';
    for (int i = 0; i < str.length(); i++)
    {
      message[i] = str[i]; // copy string to char array
    }
    accountLogs[accountID].lock_write();   // lock account log
    *file << log;                          // write log to file
    accountLogs[accountID].unlock_write(); // unlock account log
    recordSucc(message);                   // log success
    accounts[accountID].unlock_write();    // unlock account
    return 0;                              // return 0
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
 * @param file the file to write the log to
 * @return int 0 on success -1 on failure
 */
int Bank::withdraw(int workerID, int ledgerID, int accountID, int amount, fstream *file)
{
  accounts[accountID].lock_write();                        // lock account
  if (accounts[accountID].balance > amount && amount >= 0) // check if balance is greater than amount
  {
    accounts[accountID].balance -= amount;                                                                                                                                   // subtract amount from balance
    string str = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": withdraw " + to_string(amount) + " from account " + to_string(accountID); // create log message
    *file << "Transaction Type: Withdraw, Amount: " + to_string(amount) + ", Status: Success" << endl;                                                                       // write log to file
    char message[str.length() + 1];                                                                                                                                          // create char array
    message[str.length()] = '\0';                                                                                                                                            // set last char to null
    for (int i = 0; i < str.length(); i++)
    {
      message[i] = str[i]; // copy string to char array
    }
    string log = "Transaction Type: Withdraw, Amount: 0, Status: Failed\n"; // create log message
    accountLogs[accountID].lock_write();                                    // lock account log
    *file << log;                                                           // write log to file
    accountLogs[accountID].unlock_write();                                  // unlock account log
    recordSucc(message);                                                    // log success
    accounts[accountID].unlock_write();                                     // unlock account
  }
  else
  {
    string str = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": withdraw " + to_string(amount) + " from account " + to_string(accountID); // create log message
    char message[str.length() + 1];                                                                                                                                                   // create char array
    message[str.length()] = '\0';                                                                                                                                                     // set last char to null
    for (int i = 0; i < str.length(); i++)
    {
      message[i] = str[i]; // copy string to char array
    }
    string log = "Transaction Type: Withdraw, Amount: 0, Status: Failed\n"; // create log message
    accountLogs[accountID].lock_write();                                    // lock account log
    *file << log;                                                           // write log to file
    accountLogs[accountID].unlock_write();                                  // unlock account log
    recordFail(message);                                                    // log failure
    accounts[accountID].unlock_write();                                     // unlock account
    return -1;                                                              // return -1
  }
  return 0; // return 0
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
 * @param file the file to write the log to (for the source account)
 * @param file2 the file to write the log to (for the destination account)
 * @return int 0 on success -1 on error
 */
int Bank::transfer(int workerID, int ledgerID, int srcID, int destID,
                   unsigned int amount, fstream *file, fstream *file2)
{
  accounts[srcID].lock_write();                                           // lock source account
  if (accounts[srcID].balance > amount && srcID != destID && amount >= 0) // check if source account has enough money
  {
    accounts[srcID].balance -= amount;                                                                                                                                                                        // subtract amount from source account
    accounts[destID].lock_write();                                                                                                                                                                            // lock destination account
    accounts[destID].balance += amount;                                                                                                                                                                       // add amount to destination account
    accounts[destID].unlock_write();                                                                                                                                                                          // unlock destination account
    string str = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": transfer " + to_string(amount) + " from account " + to_string(srcID) + " to account " + to_string(destID); // create log message
    char message[str.length() + 1];                                                                                                                                                                           // create char array
    message[str.length()] = '\0';                                                                                                                                                                             // set last char to null
    for (int i = 0; i < str.length(); i++)                                                                                                                                                                    // copy string to char array
    {
      message[i] = str[i]; // copy string to char array
    }
    recordSucc(message);                                                                                                                   // log success
    string log1 = "Transaction Type: Transfer, Amount: " + to_string(amount) + ", Receiver: " + to_string(destID) + ", Status: Success\n"; // create log message
    string log2 = "Transaction Type: Transfer, Amount: " + to_string(amount) + ", Sender: " + to_string(srcID) + ", Status: Success\n";    // create log message
    accountLogs[srcID].lock_write();                                                                                                       // lock source account log
    *file << log1;                                                                                                                         // write log to file
    accountLogs[srcID].unlock_write();                                                                                                     // unlock source account log
    accountLogs[destID].lock_write();                                                                                                      // lock destination account log
    *file2 << log2;                                                                                                                        // write log to file
    accountLogs[destID].unlock_write();                                                                                                    // unlock destination account log
    accounts[srcID].unlock_write();                                                                                                        // unlock source account
  }
  else
  {
    string str = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": transfer " + to_string(amount) + " from account " + to_string(srcID) + " to account " + to_string(destID); // create log message
    char message[str.length() + 1];                                                                                                                                                                                    // create char array
    message[str.length()] = '\0';                                                                                                                                                                                      // set last char to null
    for (int i = 0; i < str.length(); i++)                                                                                                                                                                             // copy string to char array
    {
      message[i] = str[i]; // copy string to char array
    }
    recordFail(message);
    string log1 = "Transaction Type: Transfer, Amount: 0, Receiver: " + to_string(destID) + ", Status: Failed\n"; // create log message
    string log2 = "Transaction Type: Transfer, Amount: 0, Sender: " + to_string(srcID) + ", Status: Failed\n";    // create log message
    accountLogs[srcID].lock_write();                                                                              // lock source account log
    *file << log1;                                                                                                // write log to file
    accountLogs[srcID].unlock_write();                                                                            // unlock source account log
    accountLogs[destID].lock_write();                                                                             // lock destination account log
    *file2 << log2;                                                                                               // write log to file
    accountLogs[destID].unlock_write();                                                                           // unlock destination account log
    accounts[srcID].unlock_write();                                                                               // unlock source account
    return -1;                                                                                                    // return -1
  }
  return 0; // return 0
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
 * @param file the file to write the log to
 * @return int 0 on success -1 on error
 */
int Bank::check_balance(int workerID, int ledgerID, int accountID, fstream *file)
{
  accounts[accountID].lock_read();                                                                                                                  // lock account
  int balance = accounts[accountID].balance;                                                                                                        // get balance
  pthread_mutex_lock(&bank_lock);                                                                                                                   // lock bank
  cout << "Account " << accountID << " - Balance: " << balance << endl;                                                                             // print balance
  pthread_mutex_unlock(&bank_lock);                                                                                                                 // unlock bank
  string str = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": check balance of account " + to_string(accountID); // create log message
  char message[str.length() + 1];                                                                                                                   // create char array
  message[str.length()] = '\0';                                                                                                                     // set last char to null
  for (int i = 0; i < str.length(); i++)
  {
    message[i] = str[i]; // copy string to char array
  }
  recordSucc(message);                                                          // log success
  string log = "Transaction Type: Check Balance, Amount: 0, Status: Success\n"; // create log message
  accountLogs[accountID].lock_write();                                          // lock account log
  *file << log;                                                                 // write log to file
  accountLogs[accountID].unlock_write();                                        // unlock account log
  accounts[accountID].unlock_read();                                            // unlock account
  return 0;
}

/**
 * @brief Prints the transaction log of an account
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
int Bank::printAccountLog(int workerID, int ledgerID, int accountID, fstream *file)
{
  string line;           // string to hold each line of the file
  if ((*file).is_open()) // if the file is open
  {
    accountLogs[accountID].lock_read(); // lock account log
    while (getline(*file, line))        // while there are lines to read
    {
      accountLogs[accountID].unlock_read(); // unlock account log
      pthread_mutex_lock(&bank_lock);       // lock bank
      cout << line << '\n';                 // print line
      pthread_mutex_unlock(&bank_lock);     // unlock bank
      accountLogs[accountID].lock_read();   // lock account log
    }
    accountLogs[accountID].unlock_read(); // unlock account log
  }
  else
  {
    string str = "Worker " + to_string(workerID) + " failed to completed ledger " + to_string(ledgerID) + ": print account log of account " + to_string(accountID); // create log message
    char message[str.length() + 1];                                                                                                                                 // create char array
    message[str.length()] = '\0';                                                                                                                                   // set last char to null
    for (int i = 0; i < str.length(); i++)                                                                                                                          // copy string to char array
    {
      message[i] = str[i]; // copy string to char array
    }
    recordFail(message); // log error
    return -1;           // Indicate failure
  }
  string str = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": print account log of account " + to_string(accountID); // create log message
  char message[str.length() + 1];                                                                                                                       // create char array
  message[str.length()] = '\0';                                                                                                                         // set last char to null
  for (int i = 0; i < str.length(); i++)                                                                                                                // copy string to char array
  {
    message[i] = str[i]; // copy string to char array
  }
  recordSucc(message); // log success
  return 0;            // Indicate success
}
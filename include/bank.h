#ifndef _BANK_H
#define _BANK_H

#include <stdlib.h>
#include <fstream>
#include <string>
#include <sys/wait.h>  /* for wait() */
#include <stdlib.h>    /* for atoi() and exit() */
#include <sys/mman.h>  /* for mmap() ) */
#include <semaphore.h> /* for sem */
#include <assert.h>    /* for assert */
#include <iostream>    /* for cout */
#include <list>
#include <array>
#include <pthread.h>

using namespace std;

struct Account
{
  unsigned int accountID;
  long balance;
  int read_count;
  pthread_mutex_t read_lock, write_lock;
  void lock_read()
  {
    pthread_mutex_lock(&read_lock);
    read_count++;
    if (read_count == 1)
    {
      pthread_mutex_lock(&write_lock);
    }
    pthread_mutex_unlock(&read_lock);
  }
  void unlock_read()
  {
    pthread_mutex_lock(&read_lock);

    read_count--;
    if (read_count == 0)
    {
      pthread_mutex_unlock(&write_lock);
    }
    pthread_mutex_unlock(&read_lock);
  }
  void lock_write()
  {
    pthread_mutex_lock(&write_lock);
  }
  void unlock_write()
  {
    pthread_mutex_unlock(&write_lock);
  }
}

;

class Bank
{
private:
  int num;
  int num_succ;
  int num_fail;

public:
  Bank(int N);
  ~Bank(); // destructor

  int deposit(int workerID, int ledgerID, int accountID, int amount, fstream *file);
  int withdraw(int workerID, int ledgerID, int accountID, int amount, fstream *file);
  int transfer(int workerID, int ledgerID, int src_id, int dest_id, unsigned int amount, fstream *file, fstream *file2);
  int check_balance(int workerID, int ledgerID, int accountID, fstream *file);
  int printAccountLog(int workerID, int ledgerID, int accountID, fstream *file);

  void print_account();
  void recordSucc(char *message);
  void recordFail(char *message);

  pthread_mutex_t bank_lock;
  struct Account *accounts;
};

#endif
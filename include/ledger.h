#ifndef _LEDGER_H
#define _LEDGER_H

#include <bank.h>

using namespace std;

// Constants representing transaction modes
#define D 0
#define W 1
#define T 2
#define C 3
#define P 4

// Seed for random number generation
const int SEED_RANDOM = 377;

// Structure representing a ledger entry
struct Ledger
{
	int acc;	  // Account ID
	int other;	  // Other account ID (for transfer)
	int amount;	  // Transaction amount
	int mode;	  // Transaction mode (Deposit, Withdrawal, Transfer, Check Balance, Print Account Log)
	int ledgerID; // Ledger entry ID
};

// External declaration of the ledger list
extern list<struct Ledger> ledger;

// Function to initialize the bank and set up worker threads
void InitBank(int num_workers, char *filename);

// Function to parse a ledger file and store each line into the ledger list
void load_ledger(char *filename);

// Worker thread function
void *worker(void *unused);

#endif

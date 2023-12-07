#include <ledger.h>

using namespace std;

pthread_mutex_t ledger_lock; // mutex for ledger

list<struct Ledger> ledger; // list of ledger entries
Bank *bank;					// bank object
fstream myfile[10];			// log files

/**
 * @brief creates a new bank object and sets up workers
 *
 * Requirements:
 *  - Create a new Bank object class with 10 accounts.
 *  - Load the ledger into a list
 *  - Set up the worker threads.
 *
 * @param num_workers
 * @param filename
 */
void InitBank(int num_workers, char *filename)
{
	bank = new Bank(10);					 // create a new bank object with 10 accounts
	bank->print_account();					 // print the initial account balances
	load_ledger(filename);					 // load the ledger into a list
	pthread_t threads[num_workers];			 // create an array of threads
	int workerID[num_workers];				 // create an array of worker IDs
	ledger_lock = PTHREAD_MUTEX_INITIALIZER; // initialize the ledger lock
	for (int i = 0; i < 10; ++i)
	{
		string filename = "log_account_" + to_string(i) + ".txt"; // create a log file for each account
		myfile[i].open(filename);								  // open the log file
		if (!myfile[i].is_open())								  // check if the file is open
		{
			cerr << "Error opening log file for account " << i << endl; // error if the file cannot be opened
			exit(1);													// exit the program
		}
	}

	for (int i = 0; i < num_workers; ++i)
	{
		workerID[i] = i;												  // set the worker ID
		if (pthread_create(&threads[i], NULL, worker, &workerID[i]) != 0) // create a thread for each worker
		{
			exit(1); // exit the program if the thread cannot be created
		}
	}

	for (int i = 0; i < num_workers; ++i)
	{
		if (pthread_join(threads[i], NULL) != 0) // join the threads
		{
			exit(1); // exit the program if the threads cannot be joined
		}
		else if (i == num_workers - 1)
		{
			bank->print_account();				 // print the final account balances
			pthread_mutex_destroy(&ledger_lock); // destroy the ledger lock
			delete bank;						 // delete the bank object
		}
	}

	for (int i = 0; i < 10; ++i)
	{
		myfile[i].close(); // close the log files
	}
}

/**
 * @brief Parse a ledger file and store each line into a list
 *
 * @param filename
 */
void load_ledger(char *filename)
{

	ifstream infile(filename);		   // open the ledger file
	int c, o, a, m, ledgerID = 0;	   // variables for the ledger entries
	while (infile >> c >> o >> a >> m) // read each line of the ledger file
	{
		struct Ledger l;		 // create a new ledger entry
		l.acc = c;				 // set the account number
		l.other = o;			 // set the other account number
		l.amount = a;			 // set the amount
		l.mode = m;				 // set the mode
		l.ledgerID = ledgerID++; // set the ledger ID
		ledger.push_back(l);	 // add the ledger entry to the list
	}
}

/**
 * @brief Remove items from the list and execute the instruction.
 *
 * @param workerID
 * @return void*
 */
void *worker(void *workerID)
{
	pthread_mutex_lock(&ledger_lock); // lock the ledger
	while (!ledger.empty())			  // while the ledger is not empty
	{
		Ledger entry = ledger.front();		// get the first entry in the ledger
		ledger.pop_front();					// remove the first entry from the ledger
		pthread_mutex_unlock(&ledger_lock); // unlock the ledger
		if (entry.mode == 0)				// execute the instruction
		{
			(*bank).deposit(*(int *)workerID, entry.ledgerID, entry.acc, entry.amount, &myfile[entry.acc]); // deposit
		}
		else if (entry.mode == 1) // execute the instruction
		{
			(*bank).withdraw(*(int *)workerID, entry.ledgerID, entry.acc, entry.amount, &myfile[entry.acc]); // withdraw
		}
		else if (entry.mode == 2) // execute the instruction
		{
			(*bank).transfer(*(int *)workerID, entry.ledgerID, entry.acc, entry.other, entry.amount, &myfile[entry.acc], &myfile[entry.other]); // transfer
		}
		else if (entry.mode == 3) // execute the instruction
		{
			(*bank).check_balance(*(int *)workerID, entry.ledgerID, entry.acc, &myfile[entry.acc]); // check balance
		}
		else if (entry.mode == 4) // execute the instruction
		{
			(*bank).printAccountLog(*(int *)workerID, entry.ledgerID, entry.acc, &myfile[entry.acc]); // print account log
		}
		pthread_mutex_lock(&ledger_lock); // lock the ledger
	}
	pthread_mutex_unlock(&ledger_lock); // unlock the ledger
}

#include <ledger.h>

using namespace std;

pthread_mutex_t ledger_lock;

list<struct Ledger> ledger;
Bank *bank;

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
	time_t start, end;
	time(&start);
	bank = new Bank(10);
	bank->print_account();
	load_ledger(filename);
	pthread_t threads[num_workers];
	int workerID[num_workers];
	ledger_lock = PTHREAD_MUTEX_INITIALIZER;
	for (int i = 0; i < num_workers; ++i)
	{
		workerID[i] = i;
		if (pthread_create(&threads[i], NULL, worker, &workerID[i]) != 0)
		{
			exit(1);
		}
	}

	for (int i = 0; i < num_workers; ++i)
	{
		if (pthread_join(threads[i], NULL) != 0)
		{
			exit(1);
		}
		else if (i == num_workers - 1)
		{
			bank->print_account();
			pthread_mutex_destroy(&ledger_lock);
			delete bank;
		}
	}
	time(&end);
	int time_taken = int(end - start);
	cout << "Time taken by program is: " << fixed
		 << time_taken << " sec " << endl;
}

/**
 * @brief Parse a ledger file and store each line into a list
 *
 * @param filename
 */
void load_ledger(char *filename)
{

	ifstream infile(filename);
	int c, o, a, m, ledgerID = 0;
	while (infile >> c >> o >> a >> m)
	{
		struct Ledger l;
		l.acc = c;
		l.other = o;
		l.amount = a;
		l.mode = m;
		l.ledgerID = ledgerID++;
		ledger.push_back(l);
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
	pthread_mutex_lock(&ledger_lock);
	while (!ledger.empty())
	{
		Ledger entry = ledger.front();
		ledger.pop_front();
		pthread_mutex_unlock(&ledger_lock);
		if (entry.mode == 0)
		{
			(*bank).deposit(*(int *)workerID, entry.ledgerID, entry.acc, entry.amount);
		}
		else if (entry.mode == 1)
		{
			(*bank).withdraw(*(int *)workerID, entry.ledgerID, entry.acc, entry.amount);
		}
		else if (entry.mode == 2)
		{
			(*bank).transfer(*(int *)workerID, entry.ledgerID, entry.acc, entry.other, entry.amount);
		}
		else if (entry.mode == 3)
		{
			(*bank).check_balance(*(int *)workerID, entry.ledgerID, entry.acc);
		}
		pthread_mutex_lock(&ledger_lock);
	}
	pthread_mutex_unlock(&ledger_lock);
}

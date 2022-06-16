#include <iostream>
#include <fstream>
#include <thread>
#include <random>
#include <mutex>
#include <chrono>
#include <windows.h> 
#include <stdio.h>
#include <map>
#include <cmath>
#include <algorithm>
using namespace std;

struct person {
	int value = 0;
};

const int MONEY_ARRAY_SIZE = 6;
static int money[MONEY_ARRAY_SIZE] = { 2,-2,6,-6,12,-12 }; // array with money's values allowed

mutex mtxPrint;
mutex mtx_a;
vector<thread> threads;
vector<person> queue;
vector<bool> cashiersCanFind; // shows if cashier[i] can find the people who can satisfy client's needs
int numberOfCashiers, queueSize;

bool BREAK_ALL = false;

void cashier(int number);
vector<int> findDivisors(int number);
vector<int> findPeople(int money, vector<person> fakeQueue);

// Change standard Ctrl + C event on this function
BOOL WINAPI CtrlC_Catcher(DWORD signal) {
	if (signal == CTRL_C_EVENT)
	{
		mtxPrint.lock();
		cout << "\n\nBank closes\n\n\n";
		BREAK_ALL = true;
		mtxPrint.unlock();
	}
	return TRUE;
}

// Write 2 integers in input.txt to input them in program
void input() {
	//ifstream data;
	//data.open("input.txt");
	//data >> numberOfCashiers >> queueSize;
	cin >> numberOfCashiers >> queueSize;
}

// Initialize thread for every cashier in vector of cashiers
void initializeCashierThreads() {
	vector<thread> threads1;
	threads.resize(numberOfCashiers);
	for (int i = 0; i < numberOfCashiers; i++) { threads[i] = thread(cashier, i); }
	cashiersCanFind = vector<bool>(numberOfCashiers, true);
}

// This function must be in last part of program
void waitForCashiersThreads() {
	for (int i = 0; i < numberOfCashiers; i++) { threads[i].join(); }
}

// Main logic of every cashier
void cashier(int number) {
	auto startCashier = chrono::system_clock::now();
	chrono::duration<double> workingTime = chrono::duration<double>(0);
	person currPer;
	vector<person> fakeQueue;
	bool worked = true;
	while (!BREAK_ALL)
	{
		// take a client from queue
		mtx_a.lock();
		if (queue.size() != 0 && worked) {
			currPer = queue.front();
			queue.erase(queue.begin());
			fakeQueue.assign(queue.begin(), queue.end());
		}
		else if (queue.size() != 0) {
			fakeQueue.assign(queue.begin(), queue.end());
		}
		else {
			mtx_a.unlock();
			continue;
		}
		mtx_a.unlock();


		// work with a client
		auto startClientServicing = chrono::system_clock::now();
		vector<int> suitableClients = findPeople(currPer.value, fakeQueue);
		if (suitableClients.size() == 0) {
			cashiersCanFind[number] = false;
			mtxPrint.lock();
			cout << "Cashier #" << number + 1 << " can't find a suitable client and sends a clear request\n";
			mtxPrint.unlock();
			worked = false;

		}
		else {
			sort(suitableClients.begin(), suitableClients.end(), [](int a, int b) {return a > b; });
			mtx_a.lock();
			int summ = currPer.value;
			bool indexesOk = true;
			for (int i = 0; i < suitableClients.size(); i++)
			{
				if (suitableClients[i] + 1 > queue.size()) {
					indexesOk = false;
					break;
				}
				summ += queue[suitableClients[i]].value;
			}
			if (summ == 0 && indexesOk) {
				for (int i = 0; i < suitableClients.size(); i++)
				{
					mtxPrint.lock();
					cout << "Cashier #" << number + 1 << " has deleted person with " << queue[suitableClients[i]].value << " money\n";
					mtxPrint.unlock();
					queue.erase(queue.begin() + suitableClients[i]);
				}

				mtxPrint.lock();
				cout << "Cashier #" << number + 1 << " has served main person with " << currPer.value << " money\n";
				mtxPrint.unlock();
				auto endClientServicing = chrono::system_clock::now();
				workingTime += endClientServicing - startClientServicing;
				worked = true;
			}
			else {
				worked = false;
				cashiersCanFind[number] = false;
			}
			mtx_a.unlock();
		}
	}
	auto endCashier = chrono::system_clock::now();
	chrono::duration<double> cashierTotalTime = endCashier - startCashier;
	chrono::duration<double> cashierStayTime = cashierTotalTime - workingTime;
	mtxPrint.lock();
	cout << "CASHIER #" << number + 1 << ":\n\ttotal time: " << cashierTotalTime.count() << "\n";
	cout << "\tworking time: " << workingTime.count() << "\n";
	cout << "\tstay time: " << cashierStayTime.count() << "\n\n";
	mtxPrint.unlock();
}

// Cashier uses this function to find people for client
vector<int> findPeople(int money, vector<person> fakeQueue) {
	vector<int> result;
	map<int, vector<int>> sortedClients;
	int type = 0;
	for (int i = 0; i != fakeQueue.size(); ++i) {
		int personValue = fakeQueue[i].value;
		sortedClients[personValue].push_back(i);
	}
	if (sortedClients[-money].size() != 0) { result.push_back(sortedClients[-money][0]); }
	else {
		vector<int> divisors = findDivisors(abs(money));
		sort(divisors.begin(), divisors.end(), [](int a, int b) { return a > b; }); // sort by descending
		int sum = 0;
		bool breakFor = false;
		for (auto divisor : divisors) {
			while (sortedClients[-divisor].size() != 0) {
				sum += divisor;
				result.push_back(sortedClients[-divisor].back());
				sortedClients[-divisor].pop_back();
				if (sum >= money) { breakFor = true; break; }
			}
			if (breakFor) { break; }
		}
	}
	return result;
}
// Find all divisors of number
vector<int> findDivisors(int number) {
	vector<int> divisors;
	for (int divisor = 2; divisor <= number / 2; divisor++) {
		if (number % divisor == 0) {
			divisors.push_back(divisor);
		}
	}
	return divisors;
}

// Generate a person if queue size at the moment less than queueSize
void generateQueue() {
	//things for normal random
	random_device rd; // initialise the seed of random engine
	mt19937 rng(rd()); // random engine (Mersenne-Twister)
	uniform_int_distribution<int> uni(0, MONEY_ARRAY_SIZE - 1); // chose the min and max size of random int

	while (!BREAK_ALL) {
		if (queue.size() < queueSize) {
			mtx_a.lock();
			person p;
			p.value = money[uni(rng)];
			queue.push_back(p);
			mtxPrint.lock();
			cout << "GENERATOR: generated order " << p.value << " money\n";
			mtxPrint.unlock();
			mtx_a.unlock();
		}
	}
}

// clears queue if all elements of vector cashiersCanFind are false
void clearQueue() {
	while (!BREAK_ALL) {
		if (all_of(cashiersCanFind.begin(), cashiersCanFind.end(), [](bool elem) { return !elem; })) {
			mtx_a.lock();
			queue.clear();
			cashiersCanFind = vector<bool>(numberOfCashiers, true);
			mtx_a.unlock();
			mtxPrint.lock();
			cout << "CLEARER: queue was cleared\n";
			mtxPrint.unlock();
		}
	}
}


int main() {
	if (!SetConsoleCtrlHandler(CtrlC_Catcher, TRUE)) {
		cout << "\nError: could not set control handler";
		return 1;
	}
	input();
	initializeCashierThreads();
	thread queueGenerator(generateQueue);
	thread queueClearer(clearQueue);
	waitForCashiersThreads();
	queueClearer.detach();
	queueGenerator.detach();
	return 0;
}
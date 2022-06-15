#include <iostream>
#include <fstream>
#include <thread>
#include <random>
#include <mutex>
#include <chrono>
#include <windows.h> 
#include <stdio.h>
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
vector<int> findPeople(int money, vector<person> fakeQueue);

// Change standard Ctrl + C event on this function
BOOL WINAPI CtrlC_Catcher(DWORD signal) {
	if (signal == CTRL_C_EVENT)
	{
		mtxPrint.lock();
		cout << "\n\nBank closes\n\n";
		BREAK_ALL = true;
		mtxPrint.unlock();
	}
	return TRUE;
}

// Write 2 integers in input.txt to input them in program
void input() {
//	ifstream data;
//	data.open("input.txt");
//    data >> numberOfCashiers >> queueSize;
    cin >> numberOfCashiers >> queueSize;
}


// Initialize thread for every cashier in vector of cashiers
void initializeCashierThreads() {
	threads.resize(numberOfCashiers);
	for (int i = 0; i < numberOfCashiers; i++)
	{
		threads.push_back(thread(cashier, i));
		threads[i].join();
	}
	cashiersCanFind = vector<bool>(numberOfCashiers, true);
}

// Main logic of every cashier
void cashier(int number) {
	auto startCashier = chrono::system_clock::now();
	chrono::duration<double> workTime = chrono::duration<double>(0);
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
			cout << "Cashier #" << number + 1 << " can't find a suitable client and sends a clear request";
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
					cout << number + 1 << " kass has deleted person with " << queue[suitableClients[i]].value << endl;
					mtxPrint.unlock();
					queue.erase(queue.begin() + suitableClients[i]);
				}

				mtxPrint.lock();
				cout << number + 1 << " kass has served main person with " << currPer.value << endl;
				mtxPrint.unlock();
				auto endClientServicing = chrono::system_clock::now();
				workTime += endClientServicing - startClientServicing;
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
	chrono::duration<double> cashierWorkingTime = endCashier - startCashier;
	mtxPrint.lock();
	cout << "Cashier #" << number + 1 << " total time" << cashierWorkingTime.count();
	mtxPrint.unlock();
}

vector<int> findPeople(int money, vector<person> fakeQueue) {
	vector<int> result;
	vector<vector<int>> arr(6);
	int type = 0;
	for (int i = 0; i != fakeQueue.size(); ++i) {

		switch (fakeQueue[i].value)
		{
		case 2:
			type = 0;
			break;
		case -2:
			type = 1;
			break;
		case 6:
			type = 2;
			break;
		case -6:
			type = 3;
			break;
		case 12:
			type = 4;
			break;
		default:
			type = 5;
		}
		arr[type].push_back(i);

		switch (money)
		{
		case 2:
			if (arr[1].size() != 0) result.push_back(arr[1][0]);
			break;
		case -2:
			if (arr[0].size() != 0) result.push_back(arr[0][0]);
			break;
		case 6:
			if (arr[3].size() != 0) result.push_back(arr[3][0]);
			else if (arr[1].size() >= 3) {
				result.push_back(arr[1][0]);
				result.push_back(arr[1][1]);
				result.push_back(arr[1][2]);
			}
			break;
		case -6:
			if (arr[2].size() != 0) result.push_back(arr[2][0]);
			else if (arr[0].size() >= 3) {
				result.push_back(arr[0][0]);
				result.push_back(arr[0][1]);
				result.push_back(arr[0][2]);
			}
			break;
		case 12:
			if (arr[5].size() != 0) result.push_back(arr[5][0]);
			else if (arr[3].size() >= 2) {
				result.push_back(arr[3][0]);
				result.push_back(arr[3][1]);
			}
			else if (arr[3].size() != 0 && arr[1].size() >= 3) {
				result.push_back(arr[3][0]);
				result.push_back(arr[1][0]);
				result.push_back(arr[1][1]);
				result.push_back(arr[1][2]);
			}
			else if (arr[1].size() >= 6)
			{
				for (int i = 0; i < 6; i++)
				{
					result.push_back(arr[1][i]);
				}
			}
			break;
		case -12:
			if (arr[4].size() != 0) result.push_back(arr[4][0]);
			else if (arr[2].size() >= 2) {
				result.push_back(arr[2][0]);
				result.push_back(arr[2][1]);
			}
			else if (arr[2].size() != 0 && arr[0].size() >= 3) {
				result.push_back(arr[2][0]);
				result.push_back(arr[0][0]);
				result.push_back(arr[0][1]);
				result.push_back(arr[0][2]);
			}
			else if (arr[0].size() >= 6)
			{
				for (int i = 0; i < 6; i++)
				{
					result.push_back(arr[0][i]);
				}
			}
		}
		if (result.size() != 0) {
			break;
		}
	}

	return result;
}

// 
void generateQueue() {
	//random things for queueGenerator

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
			cout << "generated " << p.value << endl;
			mtxPrint.unlock();
			mtx_a.unlock();
		}
	}
}

// 
void clearQueue() {
	while (!BREAK_ALL) {
		if (all_of(cashiersCanFind.begin(), cashiersCanFind.end(), [](bool elem) { return !elem; })) {
			mtx_a.lock();
			queue.clear();
			for (int i = 0; i < cashiersCanFind.size(); i++)
			{
				cashiersCanFind[i] = true;
			}
			mtx_a.unlock();
			mtxPrint.lock();
			cout << "\nqueue cleared\n";
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

	return 0;
}
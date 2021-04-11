#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <time.h>
using namespace std;

int USETTL = 0;

clock_t start, finish;
double  duration0, duration1, duration2, duration3;

struct item {
	int timestamp;
	string key;
	int KeySize;
	int ValueSize;
	int ClientID;
	string operation;
	int TTL;
	item() {
		timestamp = -1;
		key = "";
		KeySize = -1;
		ValueSize = -1;
		ClientID = -1;
		operation = "";
		TTL = -1;
	}
	void Print() {
		cout << timestamp << ", " << key << ", " << KeySize << ", " << ValueSize << ", " << ClientID << ", " << operation << ", " << TTL << endl;
	}
};

struct LinkedItem {
	item ITEM;
	int time;
	bool flag;
	LinkedItem* prev;
	LinkedItem* next;
	LinkedItem* upper;
	LinkedItem* lower;
	LinkedItem(item ITEM_, int time_) {
		ITEM = ITEM_;
		time = time_;
		next = NULL;
		prev = NULL;
		upper = NULL;
		lower = NULL;
		flag = false;
	}
	LinkedItem() {
		ITEM = item();
		time = -1;
		next = NULL;
		prev = NULL;
		upper = NULL;
		lower = NULL;
		flag = false;
	}
};

struct HashTable {
	bool clear;
	int size;
	int count;
	LinkedItem** table;
	BOBHash32* hashes;
	HashTable(int size_, bool clear_) {
		clear = clear_;
		size = size_;
		count = 0;
		table = new LinkedItem * [size];
		for (int i = 0; i < size; i++) {
			table[i] = NULL;
		}
		hashes = new BOBHash32;
		InitBOBHash(1, hashes);
	}

	void randomDelete() {
		srand(time(0));
		if (count < size) {
			cout << "count < size but delete" << endl;
			return;
		}
		while (true) {
			int r = rand();
			int s = r % size;
			if (table[s] == NULL)
				continue;
			count--;
			table[s] = table[s]->next;
			if (table[s])
				table[s]->prev = NULL;
			return;
		}
	}

	int insert(item ITEM, int time) {
		start = clock();
		int h = time33(ITEM.key);
		int s = h % size;
		LinkedItem* ptr = table[s];
		finish = clock();
		duration0 += (double)(finish - start) / CLOCKS_PER_SEC;

		start = clock();
		while (ptr != NULL) {
			if (ptr->ITEM.key == ITEM.key) {
				if (ptr->ITEM.timestamp + ptr->ITEM.TTL <= ITEM.timestamp && ptr->ITEM.TTL && USETTL) {
					ptr->ITEM = ITEM;
					ptr->time = time;
					ptr->flag = false;
					return -1;
				}
				int prev = ptr->time;
				if (ptr->ITEM.TTL != 0 && ITEM.TTL == 0)
					ITEM.TTL = ptr->ITEM.timestamp + ptr->ITEM.TTL - ITEM.timestamp;
				ptr->ITEM = ITEM;
				ptr->time = time;
				ptr->flag = true;
				return time - prev;
			}
			ptr = ptr->next;
		}
		finish = clock();
		duration1 += (double)(finish - start) / CLOCKS_PER_SEC;

		start = clock();
		int r = 0;
		if (count >= size && clear) {
			randomDelete();
			r = -1;
		}
		finish = clock();
		duration2 += (double)(finish - start) / CLOCKS_PER_SEC;

		start = clock();
		ptr = table[s];
		if (ptr == NULL) {
			table[s] = new LinkedItem(ITEM, time);
		}
		else {
			while (ptr->next != NULL) {
				ptr = ptr->next;
			}
			ptr->next = new LinkedItem(ITEM, time);
			ptr->next->prev = ptr;
		}
		count++;
		finish = clock();
		duration3 += (double)(finish - start) / CLOCKS_PER_SEC;

		return r;
	}

	int Coldmiss() {
		int cnt = 0;
		for (int i = 0; i < size; i++) {
			LinkedItem* ptr = table[i];
			while (ptr != NULL) {
				if (!ptr->flag)
					cnt++;
				ptr = ptr->next;
			}
		}
		return cnt;
	}

	void Print() {
		cout << "size: " << size << " count: " << count << endl;
		for (int i = 0; i < size; i++) {
			LinkedItem* ptr = table[i];
			if (ptr == NULL)
				cout << "EMPTY";
			while (ptr != NULL) {
				cout << "(" << ptr->ITEM.key << ", " << ptr->time << ") ";
				ptr = ptr->next;
			}
			cout << endl;
		}
		cout << endl;
	}
};

inline int ShowExe(string str, int T) {
	if (T % 1000000 == 0)
		cout << endl << str << T;
	if (T % 100000 == 0)
		cout << "*";
	return T;
}

int ReservoirSampling(HashTable& map, vector<item> trace, vector<int>& vec)
{
	srand(time(0));
	cout << "Reservoir: size " << map.size << endl;
	int readNum = 0;
	int writeNum = 0;
	for (int T = 0; T < trace.size(); T++) {
		ShowExe("reservoir sampling ", T);
		int r = rand();
		if ((r % (T + 1)) <= map.size)
		{
			int interv = map.insert(trace[T], T);
			if (trace[T].operation == "get")
				readNum++;
			else
				writeNum++;
			if (interv > 0 && interv < vec.size())
				vec[interv]++;
		}
	}
	cout << endl;
	vec[0] += map.Coldmiss();
	int total = 0;
	for (int i = 0; i < vec.size(); i++) {
		total += vec[i];
	}
	cout << "duration: " << duration0 << " " << duration1 << " " << duration2 << " " << duration3 << " " << endl;
	cout << "read: " << readNum << " write: " << writeNum << endl;
	cout << "find: " << total - vec[0] << endl;
	cout << "cold miss: " << vec[0] << endl;
	cout << "get " << total << " samples" << endl;
	return total;
}

struct LRUStack {
	int size;
	int count;
	LinkedItem** table;
	LinkedItem* head;
	LinkedItem* tail;
	LRUStack(int size_) {
		size = size_;
		count = 0;
		table = new LinkedItem * [size];
		for (int i = 0; i < size; i++) {
			table[i] = NULL;
		}
		head = new LinkedItem();
		tail = new LinkedItem();
		head->lower = tail;
		tail->upper = head;
	}

	void evict() {
		if (count < size) {
			cout << "count < size but delete" << endl;
			return;
		}
		if (tail->upper == NULL) {
			cout << "Evict error! Last item not found" << endl;
			return;
		}
		int h = time33(tail->upper->ITEM.key);
		int s = h % size;
		if (table[s] == tail->upper)
			table[s] = tail->upper->next;
		if (tail->upper->prev)
			tail->upper->prev->next = tail->upper->next;
		if (tail->upper->next)
			tail->upper->next->prev = tail->upper->prev;
		tail->upper = tail->upper->upper;
		tail->upper->lower = tail;
		count--;
	}

	int insert(item ITEM, int time) {
		int h = time33(ITEM.key);
		int s = h % size;
		LinkedItem* ptr = table[s];

		while (ptr != NULL) {
			if (ptr->ITEM.key == ITEM.key) {
				if (ptr->ITEM.timestamp + ptr->ITEM.TTL <= ITEM.timestamp && ptr->ITEM.TTL && USETTL) {
					ptr->ITEM = ITEM;
					ptr->time = time;
					ptr->flag = false;
					ptr->upper->lower = ptr->lower;
					ptr->lower->upper = ptr->upper;
					ptr->upper = head;
					ptr->lower = head->lower;
					head->lower->upper = ptr;
					head->lower = ptr;
					return -1;
				}
				int prev = ptr->time;
				if (ptr->ITEM.TTL != 0 && ITEM.TTL == 0)
					ITEM.TTL = ptr->ITEM.timestamp + ptr->ITEM.TTL - ITEM.timestamp;
				ptr->ITEM = ITEM;
				ptr->time = time;
				ptr->flag = true;
				ptr->upper->lower = ptr->lower;
				ptr->lower->upper = ptr->upper;
				ptr->upper = head;
				ptr->lower = head->lower;
				head->lower->upper = ptr;
				head->lower = ptr;
				return time - prev;
			}
			ptr = ptr->next;
		}

		int r = 0;
		if (count >= size) {
			evict();
			r = -1;
		}

		ptr = table[s];
		if (ptr == NULL) {
			table[s] = new LinkedItem(ITEM, time);
			table[s]->upper = head;
			table[s]->lower = head->lower;
			head->lower->upper = table[s];
			head->lower = table[s];
			
		}
		else {
			while (ptr->next != NULL) {
				ptr = ptr->next;
			}
			ptr->next = new LinkedItem(ITEM, time);
			ptr->next->prev = ptr;
			ptr = ptr->next;
			ptr->upper = head;
			ptr->lower = head->lower;
			head->lower->upper = ptr;
			head->lower = ptr;
		}
		count++;

		return r;
	}

	void PrintStack() {
		cout << "size: " << size << " count: " << count << endl;
		cout << "head" << endl;
		LinkedItem* ptr = head;
		for (int i = 0; i < count; i++) {
			ptr = ptr->lower;
			if (ptr == NULL) {
				cout << "print error, item not found" << endl;
				return;
			}
			cout << "(" << ptr->ITEM.key << ", " << ptr->time << ") " << endl;
		}
		if (ptr->lower != tail) {
			cout << "print error, last is not tail" << endl;
			cout << "(" << ptr->lower->ITEM.key << ", " << ptr->lower->time << ") " << endl;
		}
		cout << "tail" << endl << endl;
		return;
	}

	void PrintTable() {
		cout << "size: " << size << " count: " << count << endl;
		for (int i = 0; i < size; i++) {
			LinkedItem* ptr = table[i];
			if (ptr == NULL)
				cout << "EMPTY";
			while (ptr != NULL) {
				cout << "(" << ptr->ITEM.key << ", " << ptr->time << ") ";
				ptr = ptr->next;
			}
			cout << endl;
		}
		cout << endl;
		return;
	}

};

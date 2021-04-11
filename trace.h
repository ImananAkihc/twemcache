#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <time.h>
#include <string>
#include <cstring>
#include "BOBHash32.h"
#include "sample.h"
using namespace std;

#define READMAX 10000000


bool ReadItem(FILE* pf, item* ITEM) {
	char read[1000];
	char* ptr = read;
	while (fread(ptr, 1, 1, pf)) {
		if (*ptr == '\n') {
			cout << "read timestamp error" << endl;
			return false;
		}
		if (*(ptr++) == ',') {
			*(ptr - 1) = 0;
			ITEM->timestamp = atoi(read);
			memset(read, 0, 1000 * sizeof(char));
			break;
		}
	}

	ptr = read;
	while (fread(ptr, 1, 1, pf)) {
		if (*ptr == '\n') {
			cout << "read key error" << endl;
			return false;
		}
		if (*(ptr++) == ',') {
			*(ptr - 1) = 0;
			ITEM->key = (string)read;
			memset(read, 0, 1000 * sizeof(char));
			break;
		}
	}

	ptr = read;
	while (fread(ptr, 1, 1, pf)) {
		if (*ptr == '\n') {
			cout << "read KeySize error" << endl;
			return false;
		}
		if (*(ptr++) == ',') {
			*(ptr - 1) = 0;
			ITEM->KeySize = atoi(read);
			memset(read, 0, 1000 * sizeof(char));
			break;
		}
	}

	ptr = read;
	while (fread(ptr, 1, 1, pf)) {
		if (*ptr == '\n') {
			cout << "read ValueSize error" << endl;
			return false;
		}
		if (*(ptr++) == ',') {
			*(ptr - 1) = 0;
			ITEM->ValueSize = atoi(read);
			memset(read, 0, 1000 * sizeof(char));
			break;
		}
	}

	ptr = read;
	while (fread(ptr, 1, 1, pf)) {
		if (*ptr == '\n') {
			cout << "read ClientID error" << endl;
			return false;
		}
		if (*(ptr++) == ',') {
			*(ptr - 1) = 0;
			ITEM->ClientID = atoi(read);
			memset(read, 0, 1000 * sizeof(char));
			break;
		}
	}

	ptr = read;
	while (fread(ptr, 1, 1, pf)) {
		if (*ptr == '\n') {
			cout << "read operation error" << endl;
			return false;
		}
		if (*(ptr++) == ',') {
			*(ptr - 1) = 0;
			ITEM->operation = (string)read;
			memset(read, 0, 1000 * sizeof(char));
			break;
		}
	}

	ptr = read;
	while (fread(ptr, 1, 1, pf)) {
		if (*(ptr++) == '\n') {
			if (ptr - 1 == read) {
				cout << "read TTL error" << endl;
				return false;
			}
			*(ptr - 1) = 0;
			ITEM->TTL = atoi(read);
			memset(read, 0, 1000 * sizeof(char));
			break;
		}
	}

	return true;
}

int Trace(FILE* pf, vector<item>& vec, int totalflow)
{
	if (!pf) {
		cout << "file not found." << endl;
		system("pause");
		exit(-1);
	}

	item* ITEM = new item;
	while (ReadItem(pf, ITEM) && !feof(pf))
	{
		ShowExe("trace ", vec.size());
		vec.push_back(*ITEM);
		ITEM = new item;
		if (int(vec.size()) >= totalflow && totalflow != -1)
			break;
	}
	cout << endl;
	if (vec.size() != totalflow)
		cout << "reach the end of the file" << endl;
	cout << "load " << vec.size() << " items" << endl;
	return vec.size();
}

int TraceAndCount(vector<char*> path, vector<int>& rth, int ReservoirSize, int totalflow)
{
	if (path.empty())
		cout << "no files in the path" << endl;
	int n = 0;
	cout << endl << "open " << path[0] << endl;
	FILE* pf = fopen(path[0], "rb");
	vector<item> trace;

	if (totalflow != -1 && totalflow < READMAX)
		for (int i = 0; i < totalflow; i++)
			rth.push_back(0);
	else
		for (int i = 0; i < READMAX; i++)
			rth.push_back(0);

	HashTable table(ReservoirSize, 1);
	int t = totalflow;
	int cnt = 0;
	while (t != 0 && t >= -1) {
		cnt++;
		rth[0] = 0;
		cout << "cnt: " << cnt << " todo = " << t << endl;
		trace.clear();
		if (t == -1) {
			if (Trace(pf, trace, READMAX) != READMAX)
				t = 0;
		}
		else if (t > READMAX) {
			Trace(pf, trace, READMAX);
			t -= READMAX;
		}
		else {
			Trace(pf, trace, t);
			t = 0;
		}
		ReservoirSampling(table, trace, rth);
		trace.clear();
		vector<item>().swap(trace);
	}
	int total = 0;
	for (int i = 0; i < rth.size(); i++) {
		total += rth[i];
	}
	fclose(pf);
	cout << "reservoir sample " << total << " items" << endl;
	return total;
}

int TraceAndCache(vector<char*> path, vector<double>& MRC, int CacheSize, int totalflow)
{
	if (path.empty())
		cout << "no files in the path" << endl;
	int n = 0;
	cout << endl << "open " << path[0] << endl;
	FILE* pf = fopen(path[0], "rb");
	vector<item> trace;
	int miss = 0;
	int hit = 0;
	int cnt = 0;
	LRUStack cache(CacheSize);
	int t = totalflow;
	while (t != 0 && t >= -1) {
		cnt++;
		cout << "cnt: " << cnt << " todo = " << t << endl;
		trace.clear();
		if (t == -1) {
			if (Trace(pf, trace, READMAX) != READMAX)
				t = 0;
		}
		else if (t > READMAX) {
			Trace(pf, trace, READMAX);
			t -= READMAX;
		}
		else {
			Trace(pf, trace, t);
			t = 0;
		}
		for (int i = 0; i < trace.size(); i++) {
			ShowExe("insert cache ", i);
			int temp = cache.insert(trace[i], i);
			if (temp > 0)
				hit++;
			else
				miss++;
		}
		cout << endl;
		MRC.push_back(miss * 1.0 / (miss * 1.0 + hit));
		trace.clear();
		vector<item>().swap(trace);
	}
	fclose(pf);
	return totalflow;
}
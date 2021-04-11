#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include "sample.h"
using namespace std;

vector<double> AET(vector<int> rt, int total, int BufSize) {
	vector<double> P;
	P.clear();
	P.push_back(1);
	for (int i = 1; i < rt.size(); i++) {
		P.push_back(P[i - 1] - (rt[i] * 1.0 / total));
	}
	vector<double> result;
	result.clear();
	result.push_back(1);
	double sum = 0;
	int i = 0;
	for (int l = 1; l <= BufSize; l++) {
		while (sum < l && i < rt.size()) {
			sum += P[i];
			i++;
		}
		result.push_back(P[i - 1]);
	}
	return result;
}
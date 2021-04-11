#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include "trace.h"
#include "sample.h"
#include "AET.h"
using namespace std;

ofstream fp;

char cluster52[20][100] = { "/data/temp/cluster52.0", "/data/temp/cluster52.1", "/data/temp/cluster52.2", "/data/temp/cluster52.3", 
							"/data/temp/cluster52.4", "/data/temp/cluster52.5", "/data/temp/cluster52.6","/data/temp/cluster52.7", 
							"/data/temp/cluster52.8", "/data/temp/cluster52.9", "/data/temp/cluster52.10", "/data/temp/cluster52.11", 
							"/data/temp/cluster52.12", "/data/temp/cluster52.13", };
vector<char*> DefaultPath(cluster52, cluster52+13);

int main(int argc, char** argv) {
	vector<char*> path(DefaultPath);
	int TraceSize = 100000;
	int ReservoirSize = 1000;
	int CacheSize = 10000;
	fp.open("result.txt");
	for (int i = 1; i < argc; i += 2) {
		if (!strcmp(argv[i],"-p")) {
			strcpy(path[0],argv[i + 1]);
			cout << "path: " << path[0] << endl;
		}
		else if (!strcmp(argv[i], "-s")) {
			TraceSize = atoi(argv[i + 1]);
			cout << "trace size: " << TraceSize << endl;
		}
		else if (!strcmp(argv[i], "-r")) {
			ReservoirSize = atoi(argv[i + 1]);
			cout << "reservoir size: " << ReservoirSize << endl;
		}
		else if (!strcmp(argv[i], "-c")) {
			CacheSize = atoi(argv[i + 1]);
			cout << "cache size: " << CacheSize << endl;
		}
		else if (!strcmp(argv[i], "-o")) {
			fp.close();
			fp.open(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-ttl")) {
			if (!strcmp(argv[i + 1], "open"))
				USETTL = 1;
		}
		else {
			cout << "arg error" << endl;
			return -1;
		}	
	}
	vector<int> rth;
	vector<double> MR;
	/*TraceAndCache(path, MR, CacheSize, TraceSize);
	cout << "real MR: ";
	for (int i = 0; i < MR.size(); i ++) {
		fp << MR[i] << endl;
		cout << MR[i] << " ";
	}
	cout << endl;
	fp << endl;*/
	
	int total = TraceAndCount(path,rth,ReservoirSize,TraceSize);
	vector<double> MRC = AET(rth, total, CacheSize);
	cout << "AET MRC: ";
	for (int i = 0; i < int(MRC.size()); i += int(MRC.size()) / 10) {
		fp << MRC[i] << endl;
		cout << MRC[i] << " ";
	}
	cout << endl;

	return 0;
}
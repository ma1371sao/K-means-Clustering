/**
name: Shijie Ma
date: 02/11/2017
**/

extern "C" {
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
}

#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<unordered_map>
using namespace std;

class Cluster {
public:
	FILE* trainset;
	FILE* testset;
	vector<vector<double>> train;
	vector<vector<double>> test;
	vector<vector<double>> vec;
	vector<int> sumV;
	unordered_map<int, double> PC;
	unordered_map<int, vector<double>> CtoV;
	unordered_map<int, int> sumC;
	unordered_map<int, int> traintoV;
	int k;
	int errornum;
	Cluster(char* trainfile, char* testfile, int kk);
	void getTrainSet();
	void getTestSet();
	void K_Means();
	void initialize();
	int findClosest(int pos);
	void computeP();
	double testing();
};

double Cluster::testing() {
	errornum = 0;
	for (int i = 0; i < test.size(); i++) {
		int c = test[i][0];
		double min = 1000000;
		int clu = -1;
		double p = 0;
		int testclu = -1;
		double maxp = 0;
		/**
		for (int j = 0; j < k; j++) {
			if (CtoV[c][j] * PC[c] / (sumV[j] * 1.0 / train.size()) > p) {
				p = CtoV[c][j] * PC[c] / (sumV[j] * 1.0 / train.size());
				testclu = j;
			}
			double dis = sqrt((test[i][1] - vec[j][1])*(test[i][1] - vec[j][1]) + (test[i][2] - vec[j][2])*(test[i][2] - vec[j][2]));
			if (dis < min) {
				min = dis;
				clu = j;
			}
		}
		**/
		for (int j = 0; j < k; j++) {
			double dis = sqrt((test[i][1] - vec[j][1])*(test[i][1] - vec[j][1]) + (test[i][2] - vec[j][2])*(test[i][2] - vec[j][2]));
			if (dis < min) {
				min = dis;
				clu = j;
			}
		}
		for (unordered_map<int, int>::iterator it = sumC.begin(); it != sumC.end(); it++) {
			if (CtoV[it->first][clu] * PC[it->first] > maxp) {
				maxp = CtoV[it->first][clu] * PC[it->first];
				testclu = it->first;
			}
		}
		if (testclu != c)
			errornum++;
	}
	//cout << "rate: " << errornum*1.0 / test.size() << endl;
	return errornum*1.0 / test.size();
}

void Cluster::computeP() {
	for (int i = 0; i < train.size(); i++) {
		int c = train[i][0];
		CtoV[c][traintoV[i]]++;
	}
	for (unordered_map<int, int>::iterator it = sumC.begin(); it != sumC.end(); it++) {
		int max = 0;
		int v = -1;
		for (int i = 0; i < k; i++) {
			CtoV[it->first][i] = CtoV[it->first][i] / it->second;
			if (CtoV[it->first][i] > max) {
				max = CtoV[it->first][i];
				v = i;
			}
			cout << "P(" << i << "|" << it->first << ")= " << CtoV[it->first][i] << endl;
		}
		PC[it->first] = it->second*1.0 / train.size();
		cout << "P(" << it->first << ")= " << PC[it->first] << endl;
	}
}

int Cluster::findClosest(int pos) {
	double min = 1000000;
	int clu = -1;
	for (int i = 0; i < k; i++) {
		double dis = sqrt((train[pos][1] - vec[i][1])*(train[pos][1] - vec[i][1]) + (train[pos][2] - vec[i][2])*(train[pos][2] - vec[i][2]));
		if (dis < min) {
			min = dis;
			clu = i;
		}
	}
	if (clu == -1)	cout << "error" << endl;
	return clu;
}

void Cluster::initialize() {
	int n = train.size();
	srand((unsigned)time(NULL));
	for (int i = 0; i < k; i++) {
		int t = rand() % n;
		vector<double> v;
		v.push_back(0);
		v.push_back(train[t][1]);
		v.push_back(train[t][2]);
		v.push_back(0);
		v.push_back(0);
		vec.push_back(v);
	}
}

void Cluster::K_Means() {
	initialize();
	int truen=0;
	while (truen<k) {
		for (int i = 0; i < train.size(); i++) {
			int clu = findClosest(i);
			vec[clu][0]++;
			vec[clu][3] += train[i][1];
			vec[clu][4] += train[i][2];
			traintoV[i] = clu;
		}
		truen = 0;
		for (int i = 0; i < k; i++) {
			if (vec[i][0] == 0) {
				int t = rand() % train.size();
				vec[i][1] = train[t][1];
				vec[i][2] = train[t][2];
				vec[i][3] = 0;
				vec[i][4] = 0;
			}
			else {
				sumV[i] = vec[i][0];
				double orix = vec[i][1];
				double oriy = vec[i][2];
				vec[i][1] = vec[i][3] / vec[i][0];
				vec[i][2] = vec[i][4] / vec[i][0];
				vec[i][0] = 0;
				vec[i][3] = 0;
				vec[i][4] = 0;
				//if (sqrt((vec[i][1] - orix)*(vec[i][1] - orix) + (vec[i][2] - oriy)*(vec[i][2] - oriy)) <= 0.000001)
				if(vec[i][1]==orix && vec[i][2]==oriy)
					truen++;
			}
		}
	}
	//for (int i = 0; i < k; i++)
	//	cout << vec[i][1] << " " << vec[i][2] << endl;
}

void Cluster::getTestSet() {
	char buf[1024];
	while (fgets(buf, 1023, testset)) {
		vector<double> v;
		char num[100];
		int pos = 0;
		for (int i = 0; i < strlen(buf); i++) {
			if (buf[i] != '\t')
				num[pos++] = buf[i];
			else {
				num[pos] = '\0';
				v.push_back(atof(num));
				pos = 0;
			}
		}
		num[pos] = '\0';
		v.push_back(atof(num));
		test.push_back(v);
	}
}

void Cluster::getTrainSet() {
	char buf[1024];
	while (fgets(buf, 1023, trainset)) {
		vector<double> v;
		char num[100];
		int pos = 0;
		for (int i = 0; i < strlen(buf); i++) {
			if (buf[i] != '\t')
				num[pos++] = buf[i];
			else {
				num[pos] = '\0';
				v.push_back(atof(num));
				pos = 0;
			}
		}
		num[pos] = '\0';
		v.push_back(atof(num));
		train.push_back(v);
		int c = v[0];
		sumC[c]++;
		if (CtoV.find(c) == CtoV.end()) {
			vector<double> v(k, 0);
			CtoV[c] = v;
		}
			
	}
}

Cluster::Cluster(char* trainfile, char* testfile, int kk) {
	trainset = fopen(trainfile, "r");
	testset = fopen(testfile, "r");
	k = kk;
	if (trainset == NULL) {
		fprintf(stderr, "Error opening training file %s, exiting\n", trainfile);
		exit(1);
	}
	if (testset == NULL) {
		fprintf(stderr, "Error opening testing file %s, exiting\n", testfile);
		exit(1);
	}
	for (int i = 0; i < k; i++)
		sumV.push_back(0);
	getTrainSet();
	getTestSet();
}

int main(int argc, char **argv) {
	vector<Cluster*> kmean;
	int kk;
	if (argc == 4) {
		kk = atoi(argv[3]);
		for (int i = 0; i < 10; i++)
			kmean.push_back(new Cluster(argv[1], argv[2], kk));
	}
	else {
		fprintf(stderr, "Usage: ./a.out <TrainingSetFile> <TestingSetFile> <k> > <OutputFile>\n");
		exit(1);
	}
	double errorRate[10];
	double aveER = 0;
	double SD=0;
	for (int i = 0; i < 10; i++) {
		kmean[i]->K_Means();
		kmean[i]->computeP();
		errorRate[i]=kmean[i]->testing();
		aveER += errorRate[i];
	}
	aveER /= 10;
	for (int i = 0; i < 10; i++)
		SD += (errorRate[i] - aveER)*(errorRate[i] - aveER);
	SD = sqrt(SD / 10);
	cout << "Average error rate= " << aveER << endl;
	cout << "Standard deviation= " << SD << endl;
	for (int i = 0; i < 10; i++)
		delete kmean[i];
	return 0;
}
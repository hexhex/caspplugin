#include <stdio.h>
#include <cstdlib>
#include <iostream>

using namespace std;

const int TEST_NUMBER = 10;

int main() {	
	for (int i = 0; i < TEST_NUMBER; i++) {
		char filename[5 + ((i+1)/10)];
		sprintf(filename,"%d%s",i+1, ".hex");
		freopen(filename, "w", stdout);

		cout << "$dom(0.." << (i+5) << ")." << endl;

		int task_number = 2 + i, worker_number = 2 + i + ((rand() + 1 ) % 3);

		for (int j = 0; j < task_number; j++) {
			cout << "task(" << j + 1 << "," << (rand() + 1) % 3 << ")." << endl;
		}
		cout << endl;

		for (int j = 0; j < worker_number; j++) {
			cout << "worker(" << j + 1 << "," << (rand() + 1) % (j+3) << ")." << endl;
		}
		cout << endl;

		cout << "task_assigned(X,Y) :- not task_unassigned(X,Y), task(X, Z), worker(Y,T). " << endl;
		cout << "task_unassigned(X,Y) :- not task_assigned(X,Y), task(X, Z), worker(Y,T). " << endl << endl;

		cout << "task_assigned_any(X) :- task_assigned(X,Y)." << endl << endl;

		cout << ":- not task_assigned_any(X), task(X, Y). " << endl << endl;

		cout << ":- task_assigned(X,Y1), task_assigned(X,Y2), Y1 != Y2." << endl;
		cout << ":- task_assigned(X1,Y), task_assigned(X2,Y), X1 != X2." << endl << endl;
 
		cout << "T $>= Z :- task_assigned(X,Y), task(X, Z), worker(Y,T). " << endl;
		fclose(stdout);
	}
	return 0;
}

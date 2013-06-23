#include <stdio.h>
#include <cstdlib>
#include <iostream>

using namespace std;

const int TEST_NUMBER = 20;

#define max(a,b) a>b?a:b

int main() {	
	for (int i = 0; i < TEST_NUMBER; i++) {
		char filename[11];
	
		sprintf(filename,"%d%s",i + 1, ".hex");
		freopen(filename, "w", stdout);

		int task_number = 2 + i + (rand() % (1 + i/2)), worker_number = task_number + (rand() % (1+i/2));
		int maxdomain  = 0;

		cout << endl;		
		for (int j = 0; j < task_number; j++) {
			cout << "task(" << j + 1 << "," << (rand() + 1) % (1+100*i) << ")." << endl;
		}
		cout << endl;

		for (int j = 0; j < worker_number; j++) {
			int skill = (rand() % (1+400*i));
			cout << "worker(" << j + 1 << "," << skill << ")." << endl;
			if (skill > maxdomain) maxdomain = skill;
		}

		cout << endl << "$domain(0.." << maxdomain+10 << ")." << endl;

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

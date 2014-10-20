#include <iostream>

using namespace std;

void computeQueuingAndServiceTime(double queueing, double time){
	
	ofstream queueingAndServiceTimes;
	queueingAndServiceTimes.open("Results/queueingAndServiceTimes.txt", ios::app);

	double delay;

	//cout << time << " " << queueing << endl;

	delay = time - queueing;

	queueingAndServiceTimes << delay << endl;

}
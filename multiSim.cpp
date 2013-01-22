#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include "includes/lengthOfFile.hh"

using namespace std;

struct nodesAverage{
	int num;
	double rate;
	double cp;
	float JFI;
	double bandwidth;
	double delay;
};

int main()
{
    int N = 0; //number of simulations per point
    int n_max = 0;
    int n_min = 0;
    int stickiness = 0;
    int stageStickiness = 0;
    int time = 0;
    int fairShare = 0;
    double b_min = 11000000;
    float error = 0.0;
    float drift = 0.0;
    
    //For the statistics file
    int statsLength = 0;
    nodesAverage *meanCarrier;
    

    stringstream command (stringstream::in | stringstream::out);
    
    cout << "Enter the number of simulations per point" << endl;
    cin >> N;
    
    cout << "Please introduce n_max" << endl;
    cin >> n_max;
    cout << "Please introduce n_min" << endl;
    cin >> n_min;
    cout << "How long will the simulation run? (s)" << endl;
    cin >> time;
    cout << "Initial bandwidth?" << endl;
    cin >> b_min;
    cout << "Stickiness?" << endl;
    cin >> stickiness;
    cout << "Stage Stickiness?" << endl;
    cin >> stageStickiness;
    cout << "fairShare?" << endl;
    cin >> fairShare;
    cout << "Channel error prob (%)" << endl;
    cin >> error;
    cout << "Slot drift (%)" << endl;
    cin >> drift;
    
    for(int i = n_min; i <= n_max; i++)
    {
        for(int j = 0; j < N; j++)
        {
            //execute script   
            command << "./Sim_SlottedCSMA" << " " << time << " " << i << " 1024 " << b_min << " 1 " << stickiness << " " << stageStickiness << " " << fairShare << " " << error << " " << drift  << " " << j << endl;
            //cout << command.str() << endl;
            cout << endl;
            cout << "Trying with " << i << " stations." << endl << endl;
            system(command.str().c_str());
            command.str("");
        }
    }
    
    //Manipulating the statistics.txt file
    ifstream fin("Results/multiSim.txt");
    string line;
    meanCarrier = new nodesAverage[N];
    
    statsLength = lengthOfFile(&fin);    
    
    //cout << statsLength << endl;	
    
    ifstream inputFile("Results/multiSim.txt");
    string input;
    int iterator = 0;
    
	ofstream multiAverage;
	multiAverage.open("Results/multiAverage.txt", ios::app);
    
    while(getline(inputFile,input))
    {
    	if(iterator != N-1)
    	{
    		istringstream tokenizer(input);
    		string token;
    	
	    	getline(tokenizer, token, ' ');
    		istringstream stations(token);
	    	int s;
    		stations >> s;
	    	//cout << s << endl;
    		meanCarrier[iterator].num = s;
    	
	    	getline(tokenizer, token, ' ');
    		istringstream rate(token);
	    	double r;
    		rate >> r;
	    	//cout << r << endl;
    		meanCarrier[iterator].rate = r;
    		
    		getline(tokenizer, token, ' ');
    		istringstream colProb(token);
	    	double cp;
    		colProb >> cp;
	    	//cout << cp << endl;
    		meanCarrier[iterator].cp = cp;
    		
    		getline(tokenizer, token, ' ');
    		istringstream jainsIndex(token);
	    	double jfi;
    		jainsIndex >> jfi;
	    	//cout << jfi << endl;
    		meanCarrier[iterator].JFI = jfi;
    		
    		getline(tokenizer, token, ' ');
    		istringstream bandwidth(token);
	    	double bw;
    		bandwidth >> bw;
	    	//cout << bw << endl;
    		meanCarrier[iterator].bandwidth = bw;
    		
    		getline(tokenizer, token, ' ');
    		istringstream delay(token);
	    	double sysDelay;
    		delay >> sysDelay;
	    	//cout << sysDelay << endl;
    		meanCarrier[iterator].delay = sysDelay;   		
    		
    	
    		iterator++;
    	}else
    	{
    		double numerator = 0;
    		double numerator2 = 0;
    		double numeratorJFI = 0;
    		double numeratorDelay = 0;
    		double average = 0;
    		double stDeviation = 0.0;
    		double avgJFI = 0;
    		double avgDelay = 0;
    		
    		for(int i = 0; i <= iterator; i++)
    		{
    			numerator += meanCarrier[i].rate;
    		}
    		average = numerator/iterator;
    		
    		//cout << "***Average*** = " << average << endl;
    		
    		//Computing the standard deviation for the error bars
    		//Also grabbing JFI and delay to average
    		for(int j = 0; j <= iterator; j++)
    		{	
    			if(meanCarrier[j].rate > 0)
    			{
    				numerator2 += pow(meanCarrier[j].rate - average,2);
    				numeratorJFI += (meanCarrier[j].JFI);
    				numeratorDelay += (meanCarrier[j].delay);
    			}
    		}
    		
    		stDeviation = sqrt((1./(iterator))*numerator2);
    		avgJFI = numeratorJFI/iterator;
    		avgDelay = numeratorDelay/iterator;

    		
    		multiAverage << meanCarrier[iterator-1].num << " " << average << " " << stDeviation << " " << avgJFI << " " << avgDelay << endl;
    		iterator = 0;
    		
    	}
    }//for the while statement
    
    multiAverage.close();
    	
}

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
	float backoffStage;
	double throughputDCF;
	double throughputECA;
	double sumDCFandECA;
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
    float DCF = 0.0; //0-100
    
    //For the statistics file
    //int statsLength = 0;
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
    cout << "Percentage of nodes executing DCF (%)" << endl;
    cin >> DCF;
    
    for(int i = n_min; i <= n_max; i++)
    {
        for(int j = 0; j < N; j++)
        {
            //execute script   
            command << "./Sim_SlottedCSMA" << " " << time << " " << i << " 1024 " << b_min << " 1 " << stickiness << " " << stageStickiness << " " << fairShare << " " << error/100 << " " << drift/100  << " " << DCF/100 << " " << j << endl;
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
    
    //statsLength = lengthOfFile(&fin);    
    
    //cout << statsLength << endl;	
    
    ifstream inputFile("Results/multiSim.txt");
    string input;
    int iterator = 0;
    
	ofstream multiAverage;
	multiAverage.open("Results/multiAverage.txt", ios::app);
	multiAverage << "#1-sta 2-avgThroughput 3-stdThroughput 4-JFI 5-stdJFI 6-Bandwidth 7-avgDelay 8-stdDelay 9-avgBackoffStage 10-stdBackoffStage 11-totalFractionOfCollisionSlots 12.stdCollidingSlots 13.avgThroughputDCF 14. stdThroughputDCF 15.avgThroughputECA 16.stdThroughputECA 17.Sum of 15 and 16" << endl;
    
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

    		getline(tokenizer, token, ' ');
    		istringstream backoffStage(token);
	    	double avgBackoffStage;
    		backoffStage >> avgBackoffStage;
	    	//cout << avgBackoffStage << endl;
    		meanCarrier[iterator].backoffStage = avgBackoffStage;
    		
    		getline(tokenizer, token, ' ');
    		istringstream DCF(token);
	    	double throughputDCF;
    		DCF >> throughputDCF;
	    	//cout << throughputDCF << endl;
    		meanCarrier[iterator].throughputDCF = throughputDCF;
    		
    		getline(tokenizer, token, ' ');
    		istringstream ECA(token);
	    	double throughputECA;
    		ECA >> throughputECA;
	    	//cout << throughputECA << endl;
    		meanCarrier[iterator].throughputECA = throughputECA;
    		
    		getline(tokenizer, token, ' ');
    		istringstream sum(token);
	    	double sumDCFandECA;
    		sum >> sumDCFandECA;
	    	//cout << sumDCFandECA << endl;
    		meanCarrier[iterator].sumDCFandECA = sumDCFandECA;
    	
    		iterator++;
    	}else
    	{
    		double numerator = 0;
    		double numerator2 = 0;
    		double numeratorJFI = 0;
    		double numeratorDelay = 0;
    		double numeratorSTDelay = 0;
    		double average = 0;
    		double stDeviation = 0.0;
    		double stDeviationDelay = 0.0;
    		double avgJFI = 0;
    		double stdJFI = 0;
    		double avgDelay = 0;
    		double avgBOStage = 0;
    		double numSTDBOS = 0;
    		double stdBOS = 0;
    		double avgCP = 0;
    		double numCP = 0;
    		double stdCP = 0;
    		double avgThroughputDCF = 0;
    		double avgThroughputECA = 0;
    		double stdThroughputDCF = 0;
    		double stdThroughputECA = 0;
    		double avgSumThroughput = 0;
    		double stdSumThroughput = 0;
    		
    		
    		//Computing the averages
    		for(int i = 0; i <= iterator; i++)
    		{
    			numerator += meanCarrier[i].rate;
    			numeratorJFI += (meanCarrier[i].JFI);
    			numeratorDelay += (meanCarrier[i].delay);
    			avgBOStage += (meanCarrier[i].backoffStage);
    			avgCP += (meanCarrier[i].cp);
    			avgThroughputDCF += (meanCarrier[i].throughputDCF);
    			avgThroughputECA += (meanCarrier[i].throughputECA);
    			avgSumThroughput += (meanCarrier[i].sumDCFandECA);
    		}
    		average = numerator/iterator;
    		avgJFI = numeratorJFI/iterator;
    		avgDelay = numeratorDelay/iterator;
    		avgBOStage /= iterator;
    		avgCP /= iterator;
    		avgThroughputDCF /= iterator;
    		avgThroughputECA /= iterator;
    		avgSumThroughput /= iterator;
    		
    		//Computing the standard deviation for the error bars
    		numeratorJFI = 0;
    		for(int j = 0; j <= iterator; j++)
    		{	
    			if(meanCarrier[j].rate > 0)
    			{
    				numerator2 += pow(meanCarrier[j].rate - average,2);
    				numeratorSTDelay += pow(meanCarrier[j].delay - avgDelay,2);
    				numeratorJFI += pow((meanCarrier[j].JFI) - avgJFI,2);
    				numSTDBOS += pow((meanCarrier[j].backoffStage) - avgBOStage,2);
    				numCP += pow((meanCarrier[j].cp) - avgCP,2);
    				stdThroughputDCF += pow((meanCarrier[j].throughputDCF) - avgThroughputDCF,2);
    				stdThroughputECA += pow((meanCarrier[j].throughputECA) - avgThroughputECA,2);
    				stdSumThroughput += pow((meanCarrier[j].sumDCFandECA) - avgSumThroughput,2);
    			}
    		}
    		
    		stDeviation = sqrt((1./(iterator))*numerator2);
    		stDeviationDelay = sqrt((1./(iterator))*numeratorSTDelay);
    		stdJFI = sqrt((1./(iterator))*numeratorJFI);
    		stdBOS = sqrt((1./(iterator))*numSTDBOS);
    		stdCP = sqrt((1./(iterator))*numCP);
    		stdThroughputDCF = sqrt((1./(iterator))*stdThroughputDCF);
    		stdThroughputECA = sqrt((1./(iterator))*stdThroughputECA);
    		stdSumThroughput = sqrt((1./(iterator))*stdSumThroughput);
    		    		
    		multiAverage << meanCarrier[iterator-1].num << " " << average << " " << stDeviation << " " << avgJFI << " " << stdJFI << " " << meanCarrier[iterator-1].bandwidth << " " << avgDelay << " " << stDeviationDelay << " " << avgBOStage << " " << stdBOS << " " << avgCP << " " << stdCP << " " << avgThroughputDCF << " " << stdThroughputDCF << " " << avgThroughputECA << " " << stdThroughputECA << " " << avgSumThroughput << " " << stdSumThroughput << endl;
    		iterator = 0;
    		
    	}
    }//for the while statement
    
    multiAverage.close();

}

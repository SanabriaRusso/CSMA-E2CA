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
	double Pb; //Blocking probability
	double dropped; //Dropped packets due retransmissions
	double sx; //successful slots
	double colSlots; //collision slots
	double empty; //empty slots
	double totalSlots;
	double tau;
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
    int maxAggregation = 0; //[0,1]
    
    //For the statistics file
    //int statsLength = 0;
    nodesAverage *meanCarrier;
    nodesAverage **staMeanCarrier;
    

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
    cout << "Maximum Aggregation? (0=no, 1=yes)" << endl;
    cin >> maxAggregation;
    
    for(int i = n_min; i <= n_max; i++)
    {
        for(int j = 0; j < N; j++)
        {
            //execute script   
            command << "./Sim_SlottedCSMA" << " " << time << " " << i << " 1024 " << b_min << " 1 " << stickiness << " " << stageStickiness << " " << fairShare << " " << error/100 << " " << drift/100  << " " << DCF/100 << " " << maxAggregation << " " << j << endl;
            //cout << command.str() << endl;
            cout << endl;
            cout << "Trying with " << i << " stations." << endl << endl;
            system(command.str().c_str());
            command.str("");
        }
    }
    
    //Manipulating the statistics file
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
	multiAverage << "#1-sta 2-avgThroughput 3-stdThroughput 4-JFI 5-stdJFI 6-Bandwidth 7-avgDelay 8-stdDelay 9-avgBackoffStage 10-stdBackoffStage 11-totalFractionOfCollisionSlots 12.stdCollidingSlots 13.avgThroughputDCF 14. stdThroughputDCF 15.avgThroughputECA 16.stdThroughputECA 17.Sum of 15 and 16 18. std of 17 19. AvgBlockingProb 20. StdBlockingProb 21. AvgDroppedPackets 22.StdDropped 23.SxSlots 24.stdSxSlots 25.ColSlots 26.StdColSlots 27.EmptySlots 28.stdEmpty 29.totalSlots 30.stdTotalSlots 31.averageTAU 32.stdTAU" << endl;
    
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
    		
    		getline(tokenizer, token, ' ');
    		istringstream Pb(token);
	    	double blockingProbability;
    		Pb >> blockingProbability;
	    	//cout << blockingProbability << endl;
    		meanCarrier[iterator].Pb = blockingProbability;
    		
    		getline(tokenizer, token, ' ');
    		istringstream drop(token);
	    	double droppedPackets;
    		drop >> droppedPackets;
	    	//cout << droppedPackets << endl;
    		meanCarrier[iterator].dropped = droppedPackets;
    		
    		getline(tokenizer, token, ' ');
    		istringstream sx(token);
	    	double successSlots;
    		sx >> successSlots;
	    	//cout << successSlots << endl;
    		meanCarrier[iterator].sx = successSlots;
    		
    		getline(tokenizer, token, ' ');
    		istringstream cS(token);
	    	double colSlots;
    		cS >> colSlots;
	    	//cout << colSlots << endl;
    		meanCarrier[iterator].colSlots = colSlots;
    		
    		getline(tokenizer, token, ' ');
    		istringstream empty(token);
	    	double emptySlots;
    		empty >> emptySlots;
	    	//cout << emptySlots << endl;
    		meanCarrier[iterator].empty = emptySlots;
    		
    		getline(tokenizer, token, ' ');
    		istringstream total(token);
	    	double totalSlots;
    		total >> totalSlots;
	    	//cout << totalSlots << endl;
    		meanCarrier[iterator].totalSlots = totalSlots;
    		
    		getline(tokenizer, token, ' ');
    		istringstream Pt(token);
	    	double tau;
    		Pt >> tau;
	    	//cout << tau << endl;
    		meanCarrier[iterator].tau = tau;
    	
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
    		double avgPb = 0;
    		double stdPb = 0;
    		double avgDropped = 0;
    		double stdDropped = 0;
    		double avgSxSlots = 0;
    		double stdSxSlots = 0;
    		double avgColSlots = 0;
    		double stdColSlots = 0;
    		double avgEmptySlots = 0;
    		double stdEmptySlots = 0;
    		double avgTotalSlots = 0;
    		double stdTotalSlots = 0;
    		double avgTau = 0;
    		double stdTau = 0;
    		
    		
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
    			avgPb += (meanCarrier[i].Pb);
    			avgDropped += (meanCarrier[i].dropped);
    			avgSxSlots += (meanCarrier[i].sx);
    			avgColSlots += (meanCarrier[i].colSlots);
    			avgEmptySlots += (meanCarrier[i].empty);
    			avgTotalSlots += (meanCarrier[i].totalSlots);
    			avgTau += (meanCarrier[i].tau);
    		}
    		average = numerator/iterator;
    		avgJFI = numeratorJFI/iterator;
    		avgDelay = numeratorDelay/iterator;
    		avgBOStage /= iterator;
    		avgCP /= iterator;
    		avgThroughputDCF /= iterator;
    		avgThroughputECA /= iterator;
    		avgSumThroughput /= iterator;
    		avgPb /= iterator;
    		avgDropped /= iterator;
    		avgSxSlots /= iterator;
    		avgColSlots /= iterator;
    		avgEmptySlots /= iterator;
    		avgTotalSlots /= iterator;
    		avgTau /= iterator;
    		
    		
    		
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
    				stdPb += pow((meanCarrier[j].Pb) - avgPb,2);
    				stdDropped += pow((meanCarrier[j].dropped) - avgDropped,2);
    				stdSxSlots += pow((meanCarrier[j].sx) - avgSxSlots,2);
    				stdColSlots += pow((meanCarrier[j].colSlots) - avgColSlots,2);
    				stdEmptySlots += pow((meanCarrier[j].empty) - avgEmptySlots,2);
    				stdTotalSlots += pow((meanCarrier[j].totalSlots) - avgTotalSlots,2);
    				stdTau += pow((meanCarrier[j].tau) - avgTau,2);
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
    		stdPb = sqrt((1./(iterator))*stdPb);
    		stdDropped = sqrt((1./(iterator))*stdDropped);
    		stdSxSlots = sqrt((1./(iterator))*stdSxSlots);
    		stdColSlots = sqrt((1./(iterator))*stdColSlots);
    		stdEmptySlots = sqrt((1./(iterator))*stdEmptySlots);
    		stdTotalSlots = sqrt((1./(iterator))*stdTotalSlots);
    		stdTau = sqrt((1./(iterator))*stdTau);
    		    		
    		multiAverage << meanCarrier[iterator-1].num << " " << average << " " << stDeviation << " " << avgJFI << " " << stdJFI << " " << meanCarrier[iterator-1].bandwidth << " " << avgDelay << " " << stDeviationDelay << " " << avgBOStage << " " << stdBOS << " " << avgCP << " " << stdCP << " " << avgThroughputDCF << " " << stdThroughputDCF << " " << avgThroughputECA << " " << stdThroughputECA << " " << avgSumThroughput << " " << stdSumThroughput << " " << avgPb << " " << stdPb << " " << avgDropped << " " << stdDropped << " " << avgSxSlots << " " << stdSxSlots << " " << avgColSlots << " " << stdColSlots << " " << avgEmptySlots << " " << stdEmptySlots << " " << avgTotalSlots << " " << stdTotalSlots << " " << avgTau << " " << stdTau << endl;
    		iterator = 0;
    		
    	}
    }//for the while statement
    
    multiAverage.close();
    //---------------------------------
    //Processing per nodes statistics
    //When n_max-n_min=0
    //---------------------------------
    
    //Manipulating the statistics file
    if(n_max - n_min == 0)
    {
    	nodesAverage** staMeanCarrier = new nodesAverage*[n_max];
    	for(int i = 0; i < N; i++)
    	{
    		staMeanCarrier[i] = new nodesAverage[N];
    	}
    
    	ifstream staInputFile("Results/multiStation.txt");
    
		ofstream staMultiAverage;
		staMultiAverage.open("Results/staMultiAverage.txt", ios::app);
		staMultiAverage << "#1-sta 2-avgThroughput 3-stdThroughput 4-avgPc 5-stdPc 6-avgTAU 7-stdTAU 8-avgDropped 9-stdDropped" << endl;
    
    	if(!staInputFile)
    	{
        	cout << "File could not be opened" << endl;
    	}else
    	{
    		for(int r = 0; r < N ; r++)
			{
				for(int s = 0; s < n_max; s++)
				{
            		staInputFile >> staMeanCarrier[s][r].num >> staMeanCarrier[s][r].rate;
            		//cout << staMeanCarrier[s][r].num << " " << staMeanCarrier[s][r].rate << endl;
        		}
        	}
	    }

		//Calculating the average and std
		double avgRate = 0;
		double stdRate = 0;
		
		for(int i = 0; i < n_max; i++)
		{
			for(int j = 0; j < N; j++)
			{
				avgRate += staMeanCarrier[i][j].rate;
			}
			
			avgRate /= N;
			
			staMultiAverage << i << " " << avgRate << " ";
			
			//Calculating the STD
			for(int j = 0; j < N; j++)
			{
				stdRate += pow((staMeanCarrier[i][j].rate) - avgRate,2);
			}
			stdRate = sqrt((1./(N))*stdRate);
			staMultiAverage << stdRate << endl;
			avgRate = 0;
			stdRate = 0;
		}

    	staMultiAverage.close();
    }
}

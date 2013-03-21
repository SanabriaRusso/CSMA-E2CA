#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>

//#include "/home/boris/RSoftware/sense31/code/common/cost.h"
//#include "/home/boris/Dropbox/Boris/Research/Tools/SlottedCSMA/COST/cost.h"
#include "./COST/cost.h"

#include <deque>

#include "Channel.h"
#include "STA.h"
#include "BatchPoissonSource.h"
#include "stats/stats.h"
//#include "SatNode.h"
//#include "SatNodeKenDuffy.h"

using namespace std;

component SlottedCSMA : public CostSimEng
{
	public:
		void Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int hysteresis, int fairShare, float channelErrors, float slotDrift,float percentageDCF, int simSeed);
		void Stop();
		void Start();		

	public:
		Channel channel;
//		SatNode [] stas;
		STA [] stas;
		BatchPoissonSource [] sources;

	private:
		int SimId;
		int Nodes;
		double Bandwidth_;
		int PacketLength_;
		int Batch_;
		float drift;

};

void SlottedCSMA :: Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int hysteresis, int fairShare, float channelErrors, float slotDrift, float percentageDCF, int simSeed)
{
	SimId = Sim_Id;
	Nodes = NumNodes;
	drift = slotDrift;

	stas.SetSize(NumNodes);
	sources.SetSize(NumNodes);

	// Channel	
	channel.Nodes = NumNodes;
	channel.out_slot.SetSize(NumNodes);
	channel.error = channelErrors;

	// Sat Nodes
	for(int n=0;n<NumNodes;n++)
	{
		// Node
		stas[n].node_id = n;
		stas[n].K = 1000;
		stas[n].system_stickiness = Stickiness;
		stas[n].station_stickiness = 0;
		stas[n].hysteresis = hysteresis;
		stas[n].fairShare = fairShare;
		stas[n].driftProbability = slotDrift;
		stas[n].percentageNodesWithDCF = percentageDCF;


		// Traffic Source
		sources[n].bandwidth = Bandwidth;
		sources[n].L = PacketLength;
		sources[n].MaxBatch = Batch;
	}
	
	// Connections
	for(int n=0;n<NumNodes;n++)
	{
        connect channel.out_slot[n],stas[n].in_slot;
		connect stas[n].out_packet,channel.in_packet;
		connect sources[n].out,stas[n].in_packet;

	}


	Bandwidth_ = Bandwidth;
	PacketLength_ = PacketLength;
	Batch_ = Batch;
		

};

void SlottedCSMA :: Start()
{
	printf("--------------- SlottedCSMA ---------------\n");
};

void SlottedCSMA :: Stop()
{
	double p_res = 0;
	double delay_res = 0;
	
	double overall_successful_tx = 0;
	double overall_collisions = 0;
	double overall_empty = 0;
	double total_slots = 0;
	double overall_successful_tx_slots = 0;
	double driftedSlots = 0;
	double tx_slots = 0;
	double overall_throughput = 0;
	
	

	float avg_tau = 0;
	float std_tau = 0;
	
	double stas_throughput [Nodes];
	double stas_throughputDCF [Nodes];
	double stas_throughputECA [Nodes];
	double accumThroughputDCF = 0;
	double accumThroughputECA = 0;
	
	int DCFStas = 0;
	int ECAStas = 0;
	
	double fairness_index = 0;
	double systemTXDelay = 0;
	
	//temporal statistics
	float avgBackoffStage = 0;
	//
	
	for(int n=0;n<Nodes;n++)
	{
	    avg_tau += ((float)stas[n].total_transmissions / (float)stas[n].observed_slots);
	    driftedSlots += stas[n].driftedSlots;
	    tx_slots += stas[n].total_transmissions;
	    
	    overall_successful_tx+=stas[n].successful_transmissions;
	    
	    //temporal statistics
	    avgBackoffStage += stas[n].finalBackoffStage;
	    //
	}
	overall_collisions = channel.collision_slots;
	overall_empty = channel.empty_slots;
	total_slots = channel.total_slots;
	overall_successful_tx_slots = channel.succesful_slots;
	driftedSlots /= tx_slots;

	p_res = p_res/Nodes;
	delay_res = delay_res/Nodes;
	
	avg_tau = avg_tau/Nodes;
	
	//temporal statistics
	avgBackoffStage /= Nodes;
	
	//Computing the standard deviation of each of the station's tau
	//Also capturing each station's throughput to build the Jain's index
	//And the delay of each station to derive a system average txDalay delay
	for(int i=0; i<Nodes; i++)
	{
	    std_tau += pow((float)avg_tau - ((float)stas[i].total_transmissions / (float)stas[i].observed_slots),2);
	    stas_throughput[i] = stas[i].throughput;
	    systemTXDelay += stas[i].staDelay;
	    
	    //Separating the collection of throughput of DCF and ECA stations
	    if(stas[i].DCF > 0)
	    {
	    	stas_throughputDCF[i] = stas[i].throughput;
	    	stas_throughputECA[i] = 0;
	    	DCFStas++;
	    }else
	    {
	    	stas_throughputECA[i] = stas[i].throughput;
	    	stas_throughputDCF[i] = 0;
	    	ECAStas++;
	    }
	    accumThroughputDCF += stas_throughputDCF[i];
	    accumThroughputECA += stas_throughputECA[i];
	}
	
	std_tau = pow((1.0/Nodes) * (float)std_tau, 0.5);
	systemTXDelay /= Nodes;
	
	double fair_numerator, fair_denominator;
	
	fair_numerator = 0;
	fair_denominator = 0;
	
	//computing the fairness_index
	for(int k = 0; k < Nodes; k++)
	{
	    fair_numerator += stas_throughput[k];
	    fair_denominator += pow(stas_throughput[k],2);        
	}
	
	fairness_index = (pow(fair_numerator,2)) / (Nodes*fair_denominator);
	
	//802.11n version
	overall_throughput = (channel.totalBitsSent)/SimTime();
	
	//overall_throughput = stats(overall_successful_tx, overall_empty, overall_collisions, PacketLength_);

	ofstream statistics;
	statistics.open("Results/multiSim.txt", ios::app);
	statistics << Nodes << " " << overall_throughput << " " << overall_collisions / total_slots  << " " << fairness_index  << " " << Bandwidth_ << " " << systemTXDelay << " " << avgBackoffStage << " " << accumThroughputDCF/DCFStas << " " << accumThroughputECA/ECAStas << " " << fair_numerator << endl;
	
	cout << endl << endl;
	cout << "--- Overall Statistics ---" << endl;
	cout << "Average TAU = " << avg_tau << endl;
	cout << "Standard Deviation = " << (double)std_tau << endl;
	cout << "Overall Throughput = " << overall_throughput << endl;
	
	//Need to fix this
	if((fair_numerator != (accumThroughputDCF + accumThroughputECA)) && (fair_numerator - (accumThroughputDCF+accumThroughputECA) > 1))
	{
		cout << "Error gathering the throughput of each station" << endl;
		cout << "Total: " << fair_numerator << " DCF: " << accumThroughputDCF << ", ECA: " << accumThroughputECA << ", diferring in: " << fair_numerator - (accumThroughputDCF+accumThroughputECA) << endl;
	}
	
	cout << "Jain's Fairness Index = " << fairness_index << endl;
	cout << "Overall average system TX delay = " << systemTXDelay << endl;
	cout << "Percentage of drifted slots = " << driftedSlots*100 << "%" << endl << endl;
	
	
	cout << "***Debugg***" << endl;
	cout << "Average backoff stage [0-5]: " << avgBackoffStage << endl;
	cout << "Slot drift probability: " << drift*100 << "%" << endl;
	cout << "Sx Slots: " << overall_successful_tx_slots << endl;
	cout << "Collision Slots: " << overall_collisions << endl;
	cout << "Empty Slots: " << overall_empty << endl;
	cout << "Total Slots: " << total_slots << endl;
	if(total_slots != (overall_successful_tx_slots+overall_collisions+overall_empty))
	{
	    cout << "They differ by: " << fabs(total_slots - (overall_successful_tx_slots+overall_collisions+overall_empty)) << endl;    
	}else
	{
	    cout << "They are equal" << endl;
	}
	
	cout << "Total bits sent: " << channel.totalBitsSent << " if divided by " << SimTime() << "seconds of simulation, equals = " << (channel.totalBitsSent)/SimTime() << endl << endl;

	cout << "---Debugging the mixed scenario---" << endl;
	cout << "There are: " << DCFStas << " stations with DCF and: " << ECAStas << " with CSMA/ECA." << endl;
	if(Nodes != (DCFStas + ECAStas)) cout << "Miscount of stations" << endl;
	//Avoiding divisions by zero on the cout
	if(ECAStas == 0) ECAStas = 1;
	if(DCFStas == 0) DCFStas = 1;
	cout << "The average throughput of DCF stations is: " << accumThroughputDCF/DCFStas << "bps" << endl;
	cout << "The average throughput of Full CSMA/ECA staions is: " << accumThroughputECA/ECAStas << "bps" << endl;
	if((accumThroughputECA == 0) || (accumThroughputDCF == 0))
	{
		cout << "Some stations received no throughput, so the CSMA/ECA / CSMA/CA cannot be computed" << endl;
	}else
	{
		cout << "CSMA/ECA / CSMA/CA ratio: " << (accumThroughputECA/ECAStas)/(accumThroughputDCF/DCFStas) << endl;
	}
	

};

// ---------------------------------------

int main(int argc, char *argv[])
{
	if(argc < 11) 
	{
		printf("./XXXX SimTime NumNodes PacketLength Bandwidth Batch Stickiness hysteresis fairShare channelErrors slotDrift percentageOfDCF simSeed\n");
		return 0;
	}
	
	int MaxSimIter = 1;
	double SimTime = atof(argv[1]);
	int NumNodes = atoi(argv[2]);
	int PacketLength = atoi(argv[3]);
	double Bandwidth = atof(argv[4]);
	int Batch = atoi(argv[5]); // =1
	int Stickiness = atoi(argv[6]); // 0 = DCF, up to 2.
	int hysteresis = atoi(argv[7]); //keep the current BO stage, until queue's empty
	int fairShare = atoi(argv[8]); //0 = DCF, 1 = CSMA-ECA
	float channelErrors = atof(argv[9]); // float 0-1
	float slotDrift = atof(argv[10]); // // float 0-1
	float percentageDCF = atof(argv[11]); // // float 0-1
	int simSeed = atof(argv[12]); //Simulation seed


	
	
	printf("####################### Simulation (%d) #######################\n",MaxSimIter);
	if(Stickiness > 0)
	{
		if(hysteresis > 0)
		{
			if(fairShare > 0)
			{
				cout << "####################### Full ECA #######################" << endl;
			}else
			{
				cout << "################### ECA + hysteresis ###################" << endl;
			}
		}else
		{
			cout << "###################### Basic ECA ######################" << endl;
		}
	}else
	{
		cout << "####################### CSMA/CA #######################" << endl;
	}
	
	if(percentageDCF > 0) cout << "####################### Mixed setup " << percentageDCF*100 << "% #######################" << endl;
		
	SlottedCSMA test;

	//test.Seed=(long int)6*rand();
	
	test.Seed = simSeed;
		
	test.StopTime(SimTime);

	test.Setup(MaxSimIter,NumNodes,PacketLength,Bandwidth,Batch,Stickiness, hysteresis, fairShare, channelErrors, slotDrift, percentageDCF, simSeed);
	
	test.Run();


	return(0);
};

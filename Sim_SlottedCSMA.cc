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
		void Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int stageStickiness, int fairShare);
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

};

void SlottedCSMA :: Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int stageStickiness, int fairShare)
{
	SimId = Sim_Id;
	Nodes = NumNodes;

	stas.SetSize(NumNodes);
	sources.SetSize(NumNodes);

	// Channel	
	channel.Nodes = NumNodes;
	channel.fairShare = fairShare;
	channel.out_slot.SetSize(NumNodes);

	// Sat Nodes
	for(int n=0;n<NumNodes;n++)
	{
		// Node
		stas[n].node_id = n;
		stas[n].K = 1000;
		stas[n].system_stickiness = Stickiness;
		stas[n].station_stickiness = 0;
		stas[n].stageStickiness = stageStickiness;
		stas[n].fairShare = fairShare;
		stas[n].aggregation = 1;


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
	double overall_throughput = 0;
	

	float avg_tau = 0;
	float std_tau = 0;
	
	double stas_throughput [Nodes];
	double fairness_index = 0;

	
	for(int n=0;n<Nodes;n++)
	{
	    avg_tau += ((float)stas[n].total_transmissions / (float)stas[n].observed_slots);
	    overall_successful_tx += stas[n].successful_transmissions;
	    overall_collisions += stas[n].collisions;
	    
		//p_res+=(stas[n].collisions / stas[n].total_transmissions);
		//delay_res+=(stas[n].delay / stas[n].non_blocked_packets);
		
	}
	overall_successful_tx = channel.succesful_slots;
	overall_collisions = channel.collision_slots;
	overall_empty = channel.empty_slots;
	total_slots = channel.total_slots;
	
	
	/*cout << "Success: " << overall_successful_tx << endl;
	cout << "Collisions: " << overall_collisions << endl;
	cout << "Empty: " << overall_empty << endl;*/
	
	p_res = p_res/Nodes;
	delay_res = delay_res/Nodes;
	
	avg_tau = avg_tau/Nodes;
	
	//Computing the standard deviation of each of the station's tau
	//Also capturing each station's throughput to build the Jain's index
	for(int i=0; i<Nodes; i++)
	{
	    std_tau += pow((float)avg_tau - ((float)stas[i].total_transmissions / (float)stas[i].observed_slots),2);
	    stas_throughput[i] = stas[i].throughput;
	}
	
	std_tau = pow((1.0/Nodes) * (float)std_tau, 0.5);
	
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
	overall_throughput = stats(overall_successful_tx, overall_empty, overall_collisions, PacketLength_);

	ofstream statistics;
	statistics.open("Results/statistics.txt", ios::app);
	statistics << Nodes << " " << overall_throughput << " " << overall_collisions / total_slots  << " " << fairness_index << endl;
	
	cout << endl << endl;
	cout << "--- Overall Statistics ---" << endl;
	cout << "Average TAU = " << avg_tau << endl;
	cout << "Standard Deviation = " << (double)std_tau << endl;
	cout << "Overall Throughput = " << overall_throughput << endl;
	cout << "Jain's Fairness Index = " << fairness_index << endl;
	
	

};

// ---------------------------------------

int main(int argc, char *argv[])
{
	if(argc < 2) 
	{
		printf("./XXXX SimTime NumNodes PacketLength Bandwidth Batch Stickiness stageStickiness fairShare\n");
		return 0;
	}
	
	int Stickiness;
	int stageStickiness; //keep the current BO stage, until queue's empty
	int fairShare;

	int MaxSimIter = 1;
	double SimTime = atof(argv[1]);
	int NumNodes = atoi(argv[2]);
	int PacketLength = atoi(argv[3]);
	double Bandwidth = atof(argv[4]);
	int Batch = atoi(argv[5]);
	Stickiness = atoi(argv[6]);
	stageStickiness = atoi(argv[7]);
	fairShare = atoi(argv[8]);


	printf("####################### Simulation (%d) #######################\n",MaxSimIter); 	
		
	SlottedCSMA test;

	test.Seed=(long int)6*rand();
	test.StopTime(SimTime);

	test.Setup(MaxSimIter,NumNodes,PacketLength,Bandwidth,Batch,Stickiness, stageStickiness, fairShare);
	
	test.Run();


	return(0);
};

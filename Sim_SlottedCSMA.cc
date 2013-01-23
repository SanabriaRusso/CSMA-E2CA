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
		void Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int stageStickiness, int fairShare, float channelErrors, float slotDrift, int simSeed);
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

void SlottedCSMA :: Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int stageStickiness, int fairShare, float channelErrors, float slotDrift, int simSeed)
{
	SimId = Sim_Id;
	Nodes = NumNodes;

	stas.SetSize(NumNodes);
	sources.SetSize(NumNodes);

	// Channel	
	channel.Nodes = NumNodes;
	channel.fairShare = fairShare;
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
		stas[n].stageStickiness = stageStickiness;
		stas[n].fairShare = fairShare;
		stas[n].driftProbability = slotDrift;


		// Traffic Source
		sources[n].bandwidth = Bandwidth;
		sources[n].L = PacketLength;
		sources[n].MaxBatch = Batch;
		sources[n].aggregation = fairShare;

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
	double fairness_index = 0;
	double systemTXDelay = 0;

	
	for(int n=0;n<Nodes;n++)
	{
	    avg_tau += ((float)stas[n].total_transmissions / (float)stas[n].observed_slots);
	    driftedSlots += stas[n].driftedSlots;
	    tx_slots += stas[n].total_transmissions;
	    
	    overall_successful_tx+=stas[n].successful_transmissions;	
	}
	overall_collisions = channel.collision_slots;
	overall_empty = channel.empty_slots;
	total_slots = channel.total_slots;
	overall_successful_tx_slots = channel.succesful_slots;
	driftedSlots /= tx_slots;

	p_res = p_res/Nodes;
	delay_res = delay_res/Nodes;
	
	avg_tau = avg_tau/Nodes;
	
	//Computing the standard deviation of each of the station's tau
	//Also capturing each station's throughput to build the Jain's index
	//And the delay of each station to derive a system average txDalay delay
	for(int i=0; i<Nodes; i++)
	{
	    std_tau += pow((float)avg_tau - ((float)stas[i].total_transmissions / (float)stas[i].observed_slots),2);
	    stas_throughput[i] = stas[i].throughput;
	    systemTXDelay += stas[i].staDelay;
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
	statistics << Nodes << " " << overall_throughput << " " << overall_collisions / total_slots  << " " << fairness_index  << " " << Bandwidth_ << " " << systemTXDelay << endl;
	
	cout << endl << endl;
	cout << "--- Overall Statistics ---" << endl;
	cout << "Average TAU = " << avg_tau << endl;
	cout << "Standard Deviation = " << (double)std_tau << endl;
	cout << "Overall Throughput = " << overall_throughput << endl;
	cout << "Jain's Fairness Index = " << fairness_index << endl;
	cout << "Overall average system TX delay = " << systemTXDelay << endl;
	cout << "Percentage of drifted slots = " << driftedSlots*100 << "%" << endl << endl;
	
	
	cout << "***Debugg***" << endl;
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
	
	cout << "Total bits sent: " << channel.totalBitsSent << " if divided by " << SimTime() << " equals = " << (channel.totalBitsSent)/SimTime() << endl;
	

};

// ---------------------------------------

int main(int argc, char *argv[])
{
	if(argc < 11) 
	{
		printf("./XXXX SimTime NumNodes PacketLength Bandwidth Batch Stickiness stageStickiness fairShare channelErrors slotDrift simSeed\n");
		return 0;
	}
	
	int MaxSimIter = 1;
	double SimTime = atof(argv[1]);
	int NumNodes = atoi(argv[2]);
	int PacketLength = atoi(argv[3]);
	double Bandwidth = atof(argv[4]);
	int Batch = atoi(argv[5]); // =1
	int Stickiness = atoi(argv[6]); // 0 = DCF, up to 2.
	int stageStickiness = atoi(argv[7]); //keep the current BO stage, until queue's empty
	int fairShare = atoi(argv[8]); //0 = DCF, 1 = CSMA-ECA
	int channelErrors = atof(argv[9]); // float 0-1
	int slotDrift = atof(argv[10]); // // float 0-1
	int simSeed = atof(argv[11]); //Simulation seed


	printf("####################### Simulation (%d) #######################\n",MaxSimIter); 	
		
	SlottedCSMA test;

	//test.Seed=(long int)6*rand();
	
	test.Seed = simSeed;
		
	test.StopTime(SimTime);

	test.Setup(MaxSimIter,NumNodes,PacketLength,Bandwidth,Batch,Stickiness, stageStickiness, fairShare, channelErrors, slotDrift, simSeed);
	
	test.Run();


	return(0);
};



#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#include "/home/boris/RSoftware/sense31/code/common/cost.h"
//#include "/home/boris/Dropbox/Boris/Research/Tools/SlottedCSMA/COST/cost.h"
#include "./COST/cost.h"

#include <deque>

#include "Channel.h"
#include "STA.h"
#include "BatchPoissonSource.h"
//#include "SatNode.h"
//#include "SatNodeKenDuffy.h"

component SlottedCSMA : public CostSimEng
{
	public:
		void Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch);
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

void SlottedCSMA :: Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch)
{
	SimId = Sim_Id;
	Nodes = NumNodes;

	stas.SetSize(NumNodes);
	sources.SetSize(NumNodes);

	// Channel	
	channel.Nodes = NumNodes;
	channel.out_slot.SetSize(NumNodes);

	// Sat Nodes
	for(int n=0;n<NumNodes;n++)
	{
		// Node
		stas[n].node_id = n;
		stas[n].K = 1000;


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
	for(int n=0;n<Nodes;n++)
	{
		//p_res+=(stas[n].collisions / stas[n].total_transmissions);
		//delay_res+=(stas[n].delay / stas[n].non_blocked_packets);
	}
	p_res = p_res/Nodes;
	delay_res = delay_res/Nodes;

	FILE *res;
	res = fopen("Results/res.txt","at");
	fprintf(res,"%d %d %f %d %d %f %f\n",SimId,Nodes,Bandwidth_,PacketLength_,Batch_,p_res,delay_res);
	fclose(res);
	printf("SimId Nodes Bandwidth per node Packet Length MaxBatch | Collision Prob | Total Delay\n");
	printf("%d %d %f %d %d %f %f\n",SimId,Nodes,Bandwidth_,PacketLength_,Batch_,p_res,delay_res);

};

// ---------------------------------------

int main(int argc, char *argv[])
{
	if(argc < 5) 
	{
		printf("./XXXX SimTime NumNodes PacketLength Bandwidth Batch\n");
		return 0;
	}


	int MaxSimIter = 1;
	double SimTime = atof(argv[1]);
	int NumNodes = atoi(argv[2]);
	int PacketLength = atoi(argv[3]);
	double Bandwidth = atof(argv[4]);
	int Batch = atoi(argv[5]);


	printf("####################### Simulation (%d) #######################\n",MaxSimIter); 	
		
	SlottedCSMA test;

	test.Seed=(long int)6*rand();
	test.StopTime(SimTime);

	test.Setup(MaxSimIter,NumNodes,PacketLength,Bandwidth,Batch);
	
	test.Run();


	return(0);
};

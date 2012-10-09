#include <math.h>
#include <iostream>
#include "Aux.h"
#include "FIFO.h"

#define CWMIN 32
#define MAXSTAGE 5
#define MAX_RET 7 //to be discussed

using namespace std;

component STA : public TypeII
{
    public:
        void Setup();
        void Start();
        void Stop();

    public:
        int node_id;
        int K; //max queue size
	
        double observed_slots;
        double collisions;
        double total_transmissions;
        double successful_transmissions;

        double incoming_packets;
        double non_blocked_packets;
        double blocked_packets;

        double txDelay;

    private:
        int backoff_counter;
        int backoff_stage;
        int backlogged;

        int txAttempt;	

        Packet packet;
        FIFO <Packet> MAC_queue;

    public:
        //Connections
        inport void inline in_slot(SLOT_notification &slot);
        inport void inline in_packet(Packet &packet);
        outport void out_packet(Packet &packet);
};

void STA :: Setup()
{

};

void STA :: Start()
{
    backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);
    backoff_stage = 0;
    packet.source = node_id;
    packet.L = 1000;
    packet.send_time = SimTime();

    observed_slots = 0;
    txAttempt = 0;
    collisions = 0;
    successful_transmissions = 0;
    total_transmissions = 0;

    incoming_packets = 0;
    non_blocked_packets = 0;
    blocked_packets = 0;

    txDelay = 0;

};

void STA :: Stop()
{
	cout << endl;
    cout << "--- Station " << node_id << " stats ---" << endl;
    cout << "Total Transmissions:" << " " <<  total_transmissions << endl;
    cout << "Successful Transmissions:" << " " << successful_transmissions << endl;        
    cout << "Collisions:" << " " << collisions << endl;
    cout << "*** DETAILED ***" << endl;
    cout << "TAU = " << total_transmissions / observed_slots << " |" << " p = " << collisions / total_transmissions << endl;
    cout << "Throughput = " << successful_transmissions / SimTime() << " [packets/second]" << endl;
    cout << "Blocking Probability = " << blocked_packets / incoming_packets << endl;
    cout << "Delay (queueing + service) = " << txDelay / non_blocked_packets << endl;
    cout << endl;
};

void STA :: in_slot(SLOT_notification &slot)
{
    observed_slots++;

    // print backoff values (* for stations that are not backlogged)
    // and number of packets in the queue
    if (node_id == 0)
    {
        printf("\n");
    }

    if (backlogged)
    {
        printf("%d,%d,%d\t", node_id, backoff_counter,MAC_queue.QueueSize());
    }else
    {
        printf("%d,*,%d\t", node_id,MAC_queue.QueueSize());
    }

    //stations that are backlogged will decrement backoff, transmit,
    //and check the result of the last transmission

    if (backlogged == 1)
    {
        if (slot.status == 0)
        {
            backoff_counter--;
        }
        if (slot.status == 1)
        {
            if (backoff_counter == 0) // I have transmitted
            {
                successful_transmissions++;
                txDelay += SimTime() - packet.send_time;

                MAC_queue.DelFirstPacket();

                backoff_stage = 0;
                backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);

                if (MAC_queue.QueueSize() == 0)
                {
                    backlogged = 0;
                }else
                {
                    packet = MAC_queue.GetFirstPacket();
                }
            }
        }
        
        if (slot.status > 1)
        {
            if (backoff_counter == 0) // I have transmitted
            {
                txAttempt++;
                collisions++;
                
                backoff_stage = std::min(backoff_stage+1,MAXSTAGE);
                backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);

                if (txAttempt > MAX_RET)
                {
                    txAttempt = 0;
                    backoff_stage = 0;
                    backoff_counter = (int)Random(pow(2, backoff_stage + 1)*CWMIN);
                    
                    //Grabbing a new packet and removing it from the queue
                    //The previous packet is discarded
                    packet = MAC_queue.GetFirstPacket();
                    MAC_queue.DelFirstPacket();
                    
                }
            }
        }

    }


    //stations that are not backlogged will wait for a packet
    if (backlogged == 0)
    {
        if (MAC_queue.QueueSize() > 0)
        {
            backlogged = 1;
            packet = MAC_queue.GetFirstPacket();
        }
        
    }
    //transmit if backoff counter reaches zero
    if (backoff_counter == 0)
    {
        total_transmissions++;
        out_packet(packet);
    }
};

void STA :: in_packet(Packet &packet)
{
    incoming_packets++;

    if (MAC_queue.QueueSize() < K)
    {
        non_blocked_packets++;
        MAC_queue.PutPacket(packet);
    }else
    {
        blocked_packets++;
    }
}

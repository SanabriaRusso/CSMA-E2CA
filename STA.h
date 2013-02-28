#include <math.h>
#include <iostream>
#include <fstream>
#include "Aux.h"
#include "FIFO.h"
#include "includes/backoff.hh"

#define CWMIN 16 //to comply with 802.11n. Was 32.
#define MAXSTAGE 5

//Suggested value is MAXSTAGE+1
#define MAX_RET 6


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
        int system_stickiness; //global stickiness
        int station_stickiness;
        int hysteresis;
        int fairShare;
	
        long int observed_slots;
        long int empty_slots;
        
        
        long int collisions;
        long int total_transmissions;
        long int successful_transmissions;
        long int driftedSlots;

        long int incoming_packets;
        long int non_blocked_packets;
        long int blocked_packets;

        double txDelay;
        double throughput;
        double staDelay; //overall station's delay
        
        float slotDrift;
        float driftProbability; //system slot drift probability
        
        //temporal statistics
        int finalBackoffStage;
        //

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
    packet.L = 1024;
    packet.send_time = SimTime();
    
    observed_slots = 0;
    empty_slots = 0;
    
    txAttempt = 0;
    collisions = 0;
    successful_transmissions = 0;
    total_transmissions = 0;

    incoming_packets = 0;
    non_blocked_packets = 0;
    blocked_packets = 0;

    txDelay = 0;
    
    throughput = 0;
    staDelay = 0;
    
    slotDrift = 0;
    driftedSlots = 0;
    
    //statistics
    finalBackoffStage = 0;
    //

};

void STA :: Stop()
{
    
    throughput = packet.L*8*(float)successful_transmissions / SimTime();
    
    staDelay = (float)txDelay / (float)non_blocked_packets;
    
    //temporal statistics
    finalBackoffStage = backoff_stage;
    //
    
    cout << endl;
    cout << "--- Station " << node_id << " stats ---" << endl;
    cout << "Total Transmissions:" << " " <<  total_transmissions << endl;
    cout << "Successful Transmissions:" << " " << successful_transmissions << endl;        
    cout << "Collisions:" << " " << collisions << endl;
    cout << "*** DETAILED ***" << endl;
    cout << "TAU = " << (float)total_transmissions / (float)observed_slots << " |" << " p = " << (float)collisions / (float)total_transmissions << endl;
    cout << "Throughput of this station (Boris) = " << throughput << "bps" << endl;
    cout << "Blocking Probability = " << (float)blocked_packets / (float)incoming_packets << endl;
    cout << "Delay (queueing + service) = " << staDelay << endl;
    cout << endl;
};

void STA :: in_slot(SLOT_notification &slot)
{
    observed_slots++;

    if (node_id == 0)
    {
        //cout << "Drift: " << driftProbability << endl;
    }

    if (backlogged)
    {     
        //printf("%d,%d,%d\t", node_id, backoff_counter,MAC_queue.QueueSize());
    }else
    {
       //printf("%d,*,%d\t", node_id,MAC_queue.QueueSize());
    }

    //stations that are backlogged will decrement backoff, transmit,
    //and check the result of the last transmission

    if (backlogged == 1)
    {
        if (slot.status == 0)
        {
            backoff_counter--;
            empty_slots++;
        }
        if (slot.status == 1)
        {             
            if (backoff_counter == 0) // I have transmitted
            {
                //Sent as many packets as was set in the past packet's structure
                if(fairShare > 0)
                {
                    //successful_transmissions+=(int)pow(2,backoff_stage);
                    successful_transmissions+=(int)pow(2,std::min(MAXSTAGE,MAC_queue.QueueSize()));
                }else
                {
                    successful_transmissions++;
                }

                txAttempt = 0;
                
                if(hysteresis == 0) backoff_stage = 0;
                
                txDelay += SimTime() - packet.send_time;

                //if(node_id == 0) cout << "Picket at: " << packet.send_time << ", sent at: " << SimTime() << ", with a delay of: " << SimTime() - packet.send_time << endl; 
                
                //Deleting as many packets as the aggregation field in the sent packet structure
                int qSize = MAC_queue.QueueSize();
                for(int i = 0; i <= std::min(packet.aggregation, qSize -1); i++)
                {
                    MAC_queue.DelFirstPacket();
                }
                
                //After successful tx, the sta_st goes back to sys_st
                station_stickiness = system_stickiness;
                
                backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                
                /*if(station_stickiness == 0)
                {
                    backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);    
                }else
                {  
                    backoff_counter = (int)(pow(2,backoff_stage)*CWMIN/2)-1;
                }*/
                
                //cout << "id: " << node_id << ", " << backoff_counter << endl;
               
                if (MAC_queue.QueueSize() == 0)
                {
                    backlogged = 0;
                    backoff_stage = 0;
                }else
                {
                    packet = MAC_queue.GetFirstPacket();
                    packet.send_time = SimTime();
                }
            }
            else
            {
                //Other stations transmit
                //Decrement backoff_counter
                backoff_counter--;           
            }
            
        }
        
        /*--------Collisions----------*/
        
        if (slot.status > 1)
        {   
            if (backoff_counter == 0) // I have transmitted
            {
                txAttempt++;
                collisions++;
                //One collision, one less counter stickiness
                if(system_stickiness > 0){ 
                    station_stickiness = std::max(0, station_stickiness-1);
                    if(station_stickiness == 0)
                    {
                        backoff_stage = std::min(backoff_stage+1,MAXSTAGE);
                        //backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);
                        backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                        //cout << "id: " << node_id << ", " << backoff_counter << endl;
                    }else //still sticky
                    {
                        //backoff_counter = (int)(pow(2,backoff_stage)*CWMIN/2)-1;
                        
                        //Weird scenario at the moment. Just for a system_stickiness > 1
                        backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                        //cout << "id: " << node_id << ", " << backoff_counter << endl;
                    }
                }else
                {
                    backoff_stage = std::min(backoff_stage+1,MAXSTAGE);
                    //backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);
                    backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                    //cout << "id: " << node_id << ", " << backoff_counter << endl;
                }
                    
                                                                  
                if (txAttempt > MAX_RET)
                {
                    txAttempt = 0;
                    //after dropping a frame in DCF, the backoff_stage is reset
                    if(hysteresis == 0) backoff_stage = 0;
                    
                    //Removing the number of packets as in the aggregation structure of the
                    //discarded packet, then grabbing a new one
                    if(fairShare > 0)
                    {
                        int qSize = MAC_queue.QueueSize();
                        //for(int i = 0; i <= std::min((int)pow(2,backoff_stage),qSize-1); i++)
                        for(int i = 0; i <= std::min(pow(2,MAXSTAGE),qSize-1); i++)
                        {
                            MAC_queue.DelFirstPacket();
                        }
                    }else
                    {
                        MAC_queue.DelFirstPacket();
                    }
                    packet = MAC_queue.GetFirstPacket();
                    packet.send_time = SimTime();
                    
                    //Setting the new backoff_counter
                    //backoff_counter = (int)Random(pow(2, backoff_stage + 1)*CWMIN);
                    backoff_counter = backoff(backoff_stage + 1, station_stickiness, driftProbability);
                    //cout << "id: " << node_id << ", " << backoff_counter << endl;

                }
            }
            else
            {
                //Other stations collide
                backoff_counter--;
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
            packet.send_time = SimTime();
        }
        
    }
    
    //transmit if backoff counter reaches zero
    if (backoff_counter == 0)
    {
        total_transmissions++;
        if(fairShare > 0)
        {
            //packet.aggregation = std::min((int)pow(2,backoff_stage),MAC_queue.QueueSize());
            packet.aggregation = std::min(MAXSTAGE,MAC_queue.QueueSize());
        }else
        {
            packet.aggregation = 1;
        }
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

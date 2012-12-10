#include <math.h>
#include <iostream>
#include "Aux.h"
#include "FIFO.h"

#define CWMIN 32
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
        int stageStickiness;
        int fairShare;
        int aggregation; //number of aggregated frames for transmission
	
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

};

void STA :: Stop()
{
    
    throughput = packet.L*8*(float)successful_transmissions / SimTime();
    staDelay = (float)txDelay / (float)non_blocked_packets;
    
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
        //test printf
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
                if(fairShare > 0)
                {
                    successful_transmissions+=aggregation;
                }else
                {
                    successful_transmissions++;
                }
                
                
                //Resetting transmission attempt because successful TX
                txAttempt = 0;
                
                //stageStickiness keeps backoff stage until queue is empty if set to 1
                if(stageStickiness == 0) backoff_stage = 0;
                
                txDelay += SimTime() - packet.send_time;

                //Deleting as many packets as the aggregation field in the packet structure
                if(fairShare > 0)
                {
                    //ATTENTION: the sim crashes if aggregation = Queue size
                    //Fix: aggregation should be at most Queue size -1
                    if(aggregation < MAC_queue.QueueSize())
                    {
                        for(int i = 0; i <= aggregation; i++)
                        {
                            MAC_queue.DelFirstPacket();
                        }
                    }else
                    {
                        int qSize = MAC_queue.QueueSize();
                        for(int i = 0; i < qSize; i++)
                        {
                            MAC_queue.DelFirstPacket();
                        }
                    }
                }else
                {
                    MAC_queue.DelFirstPacket();
                }
                
                //After successful tx, the sta_st goes back to sys_st
                station_stickiness = system_stickiness;
                
                if(station_stickiness == 0)
                {
                    backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);    
                }else
                {  
                    backoff_counter = (int)(pow(2,backoff_stage)*CWMIN/2)-1;
                }
               
                if (MAC_queue.QueueSize() == 0)
                {
                    backlogged = 0;
                    backoff_stage = 0;
                    //Ensures an increased JFI
                    if(fairShare > 0) aggregation = 1;
                }else
                {
                    if(fairShare > 0)
                    {
                        //If there're enough packets in Q
                        if(aggregation < MAC_queue.QueueSize())
                        {
                            packet = MAC_queue.GetFirstPacket();
                        }else //If aggregation is greater than the number of packets in Q
                        {
                            //modify the aggregation field to be the size of the Q-1?
                            aggregation = MAC_queue.QueueSize() - 1;
                            packet = MAC_queue.GetFirstPacket();
                        }
                    }else
                    {
                        packet = MAC_queue.GetFirstPacket();
                    }
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
                        backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);
                        if(fairShare > 0)
                        {//Making sure there're enough packets in the Q
                            if(pow(2,backoff_stage) < MAC_queue.QueueSize())
                            {
                                aggregation = pow(2,backoff_stage);
                            }else
                            {
                                aggregation = MAC_queue.QueueSize() - 1;
                            }
                        }
                    }else //still sticky
                    {
                        backoff_counter = (int)(pow(2,backoff_stage)*CWMIN/2)-1;
                    }
                }else
                {
                    backoff_stage = std::min(backoff_stage+1,MAXSTAGE);
                    backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);
                }
                    
                                                                  
                if (txAttempt > MAX_RET)
                {
                    txAttempt = 0;
                    //after dropping a frame, the backoff_stage is also reset
                    if(stageStickiness == 0) backoff_stage = 0;
                    //after dropping a frame, the fairness must be retained if fairShare > 0 or reset otherwise
                    if(!(fairShare > 0)) aggregation = 1;
                    
                    backoff_counter = (int)Random(pow(2, backoff_stage + 1)*CWMIN);

                    //Grabbing a new packet and removing it from the queue
                    //The previous packet is discarded
                    
                    
                    //Deleting as many packets as the aggregation field in the packet structure
                    if(fairShare > 0)
                    {
                        if(aggregation < MAC_queue.QueueSize())
                        {
                            packet = MAC_queue.GetFirstPacket();
                            for(int i = 0; i <= aggregation; i++)
                            {
                                MAC_queue.DelFirstPacket();
                            }
                        }else
                        {
                            aggregation = MAC_queue.QueueSize() - 1;
                            packet = MAC_queue.GetFirstPacket();
                            for(int i = 0; i <= aggregation; i++)
                            {
                                MAC_queue.DelFirstPacket();
                            }
                        }
                   }else
                   {
                        packet = MAC_queue.GetFirstPacket();
                        MAC_queue.DelFirstPacket();
                   }
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
        }
        
    }
    
    if(driftProbability > 0)
    {
        if(backoff_counter == 2)
        {
            slotDrift = rand() % 100 + 1;
            //if driftProbability = p, then with p/2 it will lead a slot and with p/2 it will lag a slot
            if((slotDrift > 0) && (slotDrift <= driftProbability/2.))
            {
                backoff_counter--; //lags one slot
                driftedSlots++;
            }else if((slotDrift > driftProbability/2.) && (slotDrift <= driftProbability))
            {
                backoff_counter -= 2; //leads one slot
                driftedSlots++;
            }
        }
    }
        
    
    
    //transmit if backoff counter reaches zero
    if (backoff_counter == 0)
    {
       
        //ask Jaume, because this measure has to do with TAU
        //if(fairShare > 0)
        //{
        //    total_transmissions+=aggregation;
        //}else
        //{
            total_transmissions++;
        //}
        if(fairShare > 0)
        {
            packet.aggregation = aggregation;
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

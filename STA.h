#include <math.h>
#include "Aux.h"
#include "FIFO.h"

#define CWMIN 32
#define MAXSTAGE 5

component STA : public TypeII
{
    public:
        void Setup();
        void Start();
        void Stop();

    public:
        int node_id;
        int K; //max queue size

    private:
        int backoff_counter;
        int backoff_stage;
        int backlogged;
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
};

void STA :: Stop()
{

};

void STA :: in_slot(SLOT_notification &slot)
{
    if (backlogged == 1)
    {
        if (node_id == 0)
        {
            printf("\n");
        }
        printf("%d\t", backoff_counter);
        if (slot.status == 0)
        {
            backoff_counter--;
        }
        if (slot.status == 1)
        {
            if (backoff_counter == 0) // I have transmitted
            {
                backoff_stage = 0;
                backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);
                if (MAC_queue.QueueSize() == 0)
                {
                    backlogged = 0;
                }
            }
        }
        if (slot.status > 1)
        {
            if (backoff_counter == 0) // I have transmitted
            {
                backoff_stage = std::min(backoff_stage+1,MAXSTAGE);
                backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);
            }
        }
        if (backoff_counter == 0)
        {
            out_packet(packet);
        }
    }
    if (backlogged == 0)
    {
        if (MAC_queue.QueueSize > 0)
        {
            backlogged = 1;
        }
        
    }
};

void STA :: in_packet(Packet &packet)
{
    if (MAC_queue.QueueSize() < K)
    {
        MAC_queue.PutPacket(packet);
    }
}

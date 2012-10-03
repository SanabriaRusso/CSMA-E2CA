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
      if (backoff_counter == 0); // I have transmitted
    }
};


void STA :: in_packet(Packet &packet)
{
    if (MAC_queue.QueueSize() < K)
    {
        MAC_queue.PutPacket(packet);
    }
}

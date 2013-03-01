#ifndef _AUX_
#define _AUX_

struct Packet
{
	int source;
	int destination;
	int L;
	int seq;
	double send_time; //time at which a packet is picked from the MAC queue
	int aggregation; //used whenever fairShare is activated
	double txDuration; //The duration of the transmission
};

struct SLOT_notification
{
	int status; // 0 = empty, 1 = succ. tx, >1 = collision
};


#endif 


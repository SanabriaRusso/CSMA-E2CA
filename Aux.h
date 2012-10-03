#ifndef _AUX_
#define _AUX_

struct Packet
{
	int source;
	int destination;
	int L;
	int seq;
	double send_time;
};

struct SLOT_notification
{
	int status; // 0 = empty, 1 = succ. tx, >1 = collision
};


#endif 


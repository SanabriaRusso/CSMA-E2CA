#include <math.h>
#include <iostream>

using namespace std;

double stats(int successful_slots, int empty_slots, int collision_slots, int payload){
    
    double Ts, Tc;
    double throughput;
    
    const double PACKET_PAYLOAD = 8*payload;
    const double MAC_HEADER = 272;
    const double PHY_HEADER = 128;
    const double ACK = 112 + PHY_HEADER;
    const double CHANNEL_BITRATE = 1E6;
    const double PROPAGATION_DELAY = 1E-6;
    const double SLOT_TIME = 50E-6;
    //Changing the name of DIFS and SIFS because already defined somewhere else as a global variable
    const double SIFS2 = 28E-6;
    const double DIFS2 = 128E-6;
    //double ACK_TIMEOUT = 300E-6;
    //double CTS_TIMEOUT = 300E-6;
    
    Ts = (PHY_HEADER + MAC_HEADER + PACKET_PAYLOAD)/CHANNEL_BITRATE + SIFS2 + PROPAGATION_DELAY + ACK/CHANNEL_BITRATE + DIFS2 + PROPAGATION_DELAY;
    Tc = (PHY_HEADER + MAC_HEADER + PACKET_PAYLOAD)/CHANNEL_BITRATE + DIFS2 + PROPAGATION_DELAY;
    
    throughput = successful_slots*PACKET_PAYLOAD/(empty_slots*SLOT_TIME+successful_slots*Ts+collision_slots*Tc);
    
    return(throughput);
}

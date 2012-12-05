/*
	Channel Component
*/

#define DATARATE 11E6 // Data Transmission Rate
#define PHYRATE 1E6

#define SLOT 20E-6 // Empty Slot
#define DIFS 50E-6
#define SIFS 10E-6
#define L_ack 112
			
#include "Aux.h"

component Channel : public TypeII
{

	public:
		void Setup();
		void Start();
		void Stop();
			
	public:
		int Nodes;
		int fairShare;
		float error;

		// Connections
		outport [] void out_slot(SLOT_notification &slot);	
		inport void in_packet(Packet &packet);

		// Timers
		Timer <trigger_t> slot_time; // Duration of current slot
		Timer <trigger_t> rx_time; // Time to receive all packets transmitted in current slot

		inport inline void NewSlot(trigger_t& t1);
		inport inline void EndReceptionTime(trigger_t& t2);

		Channel () { 
			connect slot_time.to_component,NewSlot; 
			connect rx_time.to_component,EndReceptionTime; }

	private:
		int number_of_transmissions_in_current_slot;
		double succ_tx_duration, collision_duration; // Depends on the packet(s) size(s)
		double L_max;
		int MAC_H, PCLP_PREAMBLE, PCLP_HEADER;
		int aggregation;
		float errorProbability;

	public: // Statistics
		double collision_slots, empty_slots, succesful_slots, total_slots;
};

void Channel :: Setup()
{

};

void Channel :: Start()
{
	number_of_transmissions_in_current_slot = 0;
	succ_tx_duration = 10E-3;
	collision_duration = 10E-3;

	collision_slots = 0;
	empty_slots = 0;
	succesful_slots = 0;
	total_slots = 0;

	L_max = 0;
	
	MAC_H = 272;
	PCLP_PREAMBLE = 144; 
	PCLP_HEADER = 48;

	aggregation = 0;
	errorProbability = 0;

	slot_time.Set(SimTime()); // Let's go!		

};

void Channel :: Stop()
{
	printf("\n\n");
	printf("---- Channel ----\n");
	printf("Slot Status Probabilities (channel point of view): Empty = %f, Succesful = %f, Collision = %f \n",empty_slots/total_slots,succesful_slots/total_slots,collision_slots/total_slots);
};

void Channel :: NewSlot(trigger_t &)
{
	//printf("%f ***** NewSlot ****\n",SimTime());

	SLOT_notification slot;

	slot.status = number_of_transmissions_in_current_slot;

	number_of_transmissions_in_current_slot = 0;
	L_max = 0;

	for(int n = 0; n < Nodes; n++) out_slot[n](slot); // We send the SLOT notification to all connected nodes

	rx_time.Set(SimTime());	// To guarantee that the system works correctly. :)
}

void Channel :: EndReceptionTime(trigger_t &)
{
	
	if(number_of_transmissions_in_current_slot==0) 
	{
		slot_time.Set(SimTime()+SLOT);
		empty_slots++;
	}
	if(number_of_transmissions_in_current_slot == 1)
	{
		slot_time.Set(SimTime()+succ_tx_duration);
		if(fairShare > 0)
		{
		    succesful_slots += aggregation;
		}else
		{
		    succesful_slots++;
		}
	}
	if(number_of_transmissions_in_current_slot > 1)
	{
		slot_time.Set(SimTime()+collision_duration);
		if(fairShare > 0)
		{
		    collision_slots += aggregation;
		}else
		{
		    collision_slots++;
		}	
	}

	if(fairShare > 0)
	{
	    total_slots+=aggregation;
	}else
	{
	    total_slots++;
	}
}


void Channel :: in_packet(Packet &packet)
{

	if(packet.L > L_max) L_max = packet.L;
	
	aggregation = packet.aggregation;
	
	errorProbability = rand() % 100 + 1;
	
	if((errorProbability > 0) && (errorProbability <= error))
	{
	    //If the channel error probability is contained inside the system error margin,
	    //then something wrong is going to happen with the transmissions in this slot
	    number_of_transmissions_in_current_slot+=2;
	}else
	{
	    number_of_transmissions_in_current_slot++;
	}
	
	//printf("Channel: %d\n",number_of_transmissions_in_current_slot);

	if(fairShare > 0)
	{
	    succ_tx_duration = ((PCLP_PREAMBLE + PCLP_HEADER)/PHYRATE) + ((aggregation*L_max*8 + MAC_H)/DATARATE) + SIFS + ((PCLP_PREAMBLE + PCLP_HEADER)/PHYRATE) + (L_ack/PHYRATE) + DIFS;
	    collision_duration = ((PCLP_PREAMBLE + PCLP_HEADER)/PHYRATE) + ((aggregation*L_max*8 + MAC_H)/DATARATE) + SIFS + DIFS + ((144 + 48 + 112)/PHYRATE);
	}else
	{
	    succ_tx_duration = ((PCLP_PREAMBLE + PCLP_HEADER)/PHYRATE) + ((L_max*8 + MAC_H)/DATARATE) + SIFS + ((PCLP_PREAMBLE + PCLP_HEADER)/PHYRATE) + (L_ack/PHYRATE) + DIFS;
	    collision_duration = ((PCLP_PREAMBLE + PCLP_HEADER)/PHYRATE) + ((L_max*8 + MAC_H)/DATARATE) + SIFS + DIFS + ((144 + 48 + 112)/PHYRATE);
	}
}


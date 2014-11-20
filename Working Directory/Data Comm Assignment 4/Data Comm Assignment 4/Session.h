#ifndef _SESSION
#define _SESSION

#include <cstdlib>
#include "Application.h"

using std::rand;

static const int ENQ_TIMEOUT_SIZE = 5;
static const int BYTE_SIZE = 8;
static const int BIT_RATE = 9600;
static const int BYTE_RATE = BIT_RATE / BYTE_SIZE;
static const int PACKET_TIMEOUT_SIZE = 1200;
static const int MAX_MISS = 3;
static const int MAX_SEND = 10;
static const int MILLISECONDS = 1000;

void calculateTimeouts( Timeouts* timeouts );
double getResetTime( Timeouts* timeouts );


/***********************************************************
 *
 * USAGE:
 *
 * Statistics* statsObject = Statistics::getInstance();
 *
 **********************************************************/
class Statistics 
{
	public:
		Statistics();
		double getSentGoodPacketPercent();
		double getSentBadPacketPercent();
		double getReceivedGoodPacketPercent();
		double getReceivedBadPacketPercent();

		void incGoodPacketSent();
		void incBadPacketSent();
		void incLostPacketSent();
		void incGoodPacketReceived();
		void incBadPacketReceived();
		void incACKSent();
		void incENQSent();
		void incNAKSent();
		void incACKReceived();
		void incENQReceived();
		void incNAKReceived();

		int getGoodPacketSent();
		int getBadPacketSent();
		int getLostPacketSent();
		int getGoodPacketReceived();
		int getBadPacketReceived();
		int getACKSent();
		int getENQSent();
		int getNAKSent();
		int getACKReceived();
		int getENQReceived();
		int getNAKReceived();

		void addPacketSentSize( char* packet );
		void addPacketReceivedSize( char* packet );
		double getAvgPacketSentSize();
		double getAvgPacketReceivedSize();

		static Statistics* getInstance();
	private:
		int sentACK;
		int sentENQ;
		int sentNAK;
		int recACK;
		int recENQ;
		int recNAK;

		int packetSentGood;
		int packetSentBad;
		int packetSentLost;
		int packetSentTotal;
		int packetReceivedGood;
		int packetReceivedBad;
		int packetReceivedTotal;

		double totalPacketSentSize;
		double totalPacketReceivedSize;

		static Statistics* instance;
};

#endif
#include <Windows.h>
#include "Session.h"

Statistics* Statistics::instance = nullptr;

Statistics::Statistics()
{
	sentACK = 0;
	sentENQ = 0;
	sentNAK = 0;
	recACK = 0;
	recENQ = 0;
	recNAK = 0;

	packetSentBad = 0;
	packetSentGood = 0;
	packetSentTotal = 0;
	packetReceivedBad = 0;
	packetReceivedGood = 0;
	packetReceivedTotal = 0;

	totalPacketReceivedSize = 0;
	totalPacketSentSize = 0;
}

int Statistics::getACKSent()
{
	return sentACK;
}

int Statistics::getACKReceived()
{
	return recACK;
}

int Statistics::getENQReceived()
{
	return sentENQ;
}

int Statistics::getENQSent()
{
	return sentENQ;
}

int Statistics::getNAKSent()
{
	return sentNAK;
}

int Statistics::getNAKReceived()
{
	return sentNAK;
}

int Statistics::getGoodPacketReceived()
{
	return packetReceivedGood;
}

int Statistics::getBadPacketReceived()
{
	return packetReceivedBad;
}

int Statistics::getGoodPacketSent()
{
	return packetSentGood;
}

int Statistics::getBadPacketSent()
{
	return packetSentBad;
}

int Statistics::getLostPacketSent()
{
	return packetSentLost;
}

double Statistics::getSentGoodPacketPercent()
{
	return double(packetSentGood) / packetSentTotal;
}

double Statistics::getSentBadPacketPercent()
{
	return double(packetSentBad) / packetSentTotal;
}

double Statistics::getReceivedGoodPacketPercent()
{
	return double(packetReceivedGood) / packetReceivedTotal;
}

double Statistics::getReceivedBadPacketPercent()
{
	return double(packetReceivedBad) / packetReceivedTotal;
}

void Statistics::incGoodPacketSent()
{
	packetSentGood++;
	packetSentTotal++;
}

void Statistics::incLostPacketSent()
{
	packetSentLost++;
	packetSentTotal++;
}


void Statistics::incBadPacketSent()
{
	packetSentBad++;
	packetSentTotal++;
}

void Statistics::incGoodPacketReceived()
{
	packetReceivedGood++;
	packetReceivedTotal++;
}

void Statistics::incBadPacketReceived()
{
	packetReceivedBad++;
	packetReceivedTotal++;
}

void Statistics::incACKReceived()
{
	recACK++;
}

void Statistics::incENQReceived()
{
	recENQ++;
}

void Statistics::incNAKReceived()
{
	recNAK++;
}

void Statistics::incACKSent()
{
	sentACK++;
}

void Statistics::incENQSent()
{
	sentENQ++;
}

void Statistics::incNAKSent()
{
	sentNAK++;
}

void Statistics::addPacketReceivedSize(char* packetData)
{
	totalPacketReceivedSize += strlen(packetData);
}

void Statistics::addPacketSentSize(char* packetData)
{
	totalPacketSentSize += strlen(packetData);
}

double Statistics::getAvgPacketReceivedSize()
{
	return double(totalPacketReceivedSize) / packetReceivedTotal;
}

double Statistics::getAvgPacketSentSize()
{
	return double(totalPacketSentSize) / packetSentTotal;
}

Statistics* Statistics::getInstance()
{
	if (instance == nullptr)
	{
		instance = new Statistics();
	}

	return instance;
}
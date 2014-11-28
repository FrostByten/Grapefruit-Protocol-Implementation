#include <Windows.h>
#include "Session.h"
#include "Application.h"

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
	packetSentLost = 0;
	packetSentTotal = 0;
	packetReceivedBad = 0;
	packetReceivedGood = 0;
	packetReceivedLost = 0;
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

int Statistics::getLostPacketReceived()
{
	return packetReceivedLost;
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

	refreshScreen();
}

void Statistics::incLostPacketSent()
{
	packetSentLost++;
	packetSentTotal++;

	refreshScreen();
}


void Statistics::incBadPacketSent()
{
	packetSentBad++;
	packetSentTotal++;

	refreshScreen();
}

void Statistics::incGoodPacketReceived()
{
	packetReceivedGood++;
	packetReceivedTotal++;

	refreshScreen();
}

void Statistics::incBadPacketReceived()
{
	packetReceivedBad++;
	packetReceivedTotal++;

	refreshScreen();
}

void Statistics::incLostPacketReceived()
{
	packetReceivedLost++;
	packetReceivedTotal++;

	refreshScreen();
}

void Statistics::incACKReceived()
{
	recACK++;

	refreshScreen();
}

void Statistics::incENQReceived()
{
	recENQ++;

	refreshScreen();
}

void Statistics::incNAKReceived()
{
	recNAK++;

	refreshScreen();
}

void Statistics::incACKSent()
{
	sentACK++;

	refreshScreen();
}

void Statistics::incENQSent()
{
	sentENQ++;

	refreshScreen();
}

void Statistics::incNAKSent()
{
	sentNAK++;

	refreshScreen();
}

void Statistics::addPacketReceivedSize(char* packetData)
{
	totalPacketReceivedSize += strlen(packetData);

	refreshScreen();
}

void Statistics::addPacketSentSize(char* packetData)
{
	totalPacketSentSize += strlen(packetData);

	refreshScreen();
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
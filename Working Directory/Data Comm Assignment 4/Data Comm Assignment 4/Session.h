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

#endif
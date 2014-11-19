#include <windows.h>
#include "Application.h"
#include "Session.h"

void calculateTimeouts( Timeouts* timeouts) {
	// Calculate each timeout in milliseconds so that they are compatible with
	// the WaitForSingleObject function
	timeouts->timeoutSendEnq = ( double(ENQ_TIMEOUT_SIZE) * BYTE_SIZE ) /  ( BIT_RATE / BYTE_SIZE ) / 100;
	timeouts->timeoutSendPacket = ( double(PACKET_TIMEOUT_SIZE) * BYTE_SIZE ) / ( BIT_RATE / BYTE_SIZE ) / 100;
	timeouts->timeoutSendAck = timeouts->timeoutSendPacket * MAX_MISS;

	return;
}
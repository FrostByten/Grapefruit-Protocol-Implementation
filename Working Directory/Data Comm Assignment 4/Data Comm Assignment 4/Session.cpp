#include <windows.h>
#include "Application.h"

void calculateTimeouts( Timeouts* timeouts) {
	// TODO: these are calculated in seconds. How do we convert this to iterations so that
	// they are comparable?
	timeouts->timeoutSendEnq = ( 5 * 8 ) /  ( 9600 / 8 );
	timeouts->timeoutSendPacket = ( 1200 * 8 ) / ( 9600 / 8 );
	timeouts->timeoutSendAck = timeouts->timeoutSendPacket * 3;

	return;
}
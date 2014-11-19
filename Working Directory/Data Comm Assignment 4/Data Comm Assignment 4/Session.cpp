/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Session.cpp - 
--
-- PROGRAM: Irregardless
--
-- FUNCTIONS:
--		void calculateTimeouts( Timeouts* timeouts);
--		double getResetTime( Timeouts* timeouts );
-- 
-- DATE: November 17, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--
-- PROGRAMMER: Christofer Klassen
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/

#include <windows.h>
#include "Application.h"
#include "Session.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: calculateTimeouts
--
-- DATE: November 19, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--
-- PROGRAMMER: Christofer Klassen
--
-- INTERFACE: void calculateTimeouts( Timeouts* timeouts );
--
-- RETURNS: void
--
-- NOTES:
-- This function calculates the timeouts for the protocol based on the preset bitrate
-- that the modem uses.
----------------------------------------------------------------------------------------------------------------------*/
void calculateTimeouts( Timeouts* timeouts) {
	// Calculate each timeout in milliseconds so that they are compatible with
	// the WaitForSingleObject function
	timeouts->timeoutSendEnq = ( ( double(ENQ_TIMEOUT_SIZE) ) /  BYTE_RATE ) * MILLISECONDS;
	timeouts->timeoutSendPacket = ( ( double(PACKET_TIMEOUT_SIZE) ) / BYTE_RATE ) * MILLISECONDS;
	timeouts->timeoutSendAck = timeouts->timeoutSendPacket * MAX_MISS;
	
	// Calculate min and max reset values in bits
	timeouts->resetMin = BYTE_SIZE;
	timeouts->resetMax = 4 * BYTE_SIZE;

	return;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getResetTime
--
-- DATE: November 19, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--
-- PROGRAMMER: Christofer Klassen
--
-- INTERFACE: double getResetTime( Timeouts* timeouts );
--
-- RETURNS: double - the length of the random timeout in milliseconds
--
-- NOTES:
-- This function calculates a random reset timeout based on the given 
-- protocol specifications and returns it.
----------------------------------------------------------------------------------------------------------------------*/
double getResetTime( Timeouts* timeouts ) {
	// Calculate a random number of bits to timeout with
	int randBits = rand() % (timeouts->resetMax - timeouts->resetMin + 1) + timeouts->resetMin;

	// Calculate and return the timeout
	return ( double(randBits) / BIT_RATE ) * MILLISECONDS;
}
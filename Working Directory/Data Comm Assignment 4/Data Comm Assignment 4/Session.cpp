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

int in_buff_place = 0;

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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: isBufferNotEmpty
--
-- DATE: November 27, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- INTERFACE: bool isBufferNotEmpty
--
-- RETURNS: bool - whether the input buffer is not empty
--
-- NOTES:
-- This function checks the input buffer. If it is empty, it returns false, otherwise, it returns true.
----------------------------------------------------------------------------------------------------------------------*/
bool isBufferNotEmpty()
{
	if (sendBuffer[0] == '\0')
		return false;
	else
		return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: pushPacketToDisplayBuffer
--
-- DATE: November 27, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- INTERFACE: void pushPacketToDisplayBuffer(unsigned char *pack)
--
-- RETURNS: void
--
-- NOTES:
-- This function pushes the data contained within a packet to the display buffer for rendering
----------------------------------------------------------------------------------------------------------------------*/
void pushPacketToDisplayBuffer(unsigned char *pack)
{
	for (int i = DATA_START; i < DATA_END; i++)
	{
		if (pack[i] == ETX)
		{
			in_buff_place++;
			break;
		}
		printText[in_buff_place] = pack[i];
		in_buff_place++;
	}

	printText[in_buff_place] = '\0';
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: popFromBuffer
--
-- DATE: November 27, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- INTERFACE: void popFromBuffer(int count)
--
-- RETURNS: void
--
-- NOTES:
-- This function pops a certain length of data from the input buffer, then slides the remaining data down
-- to the front.
----------------------------------------------------------------------------------------------------------------------*/
void popFromBuffer(int count)
{
	for (int i = count; i < 1024; i++)
	{
		sendBuffer[i - count] = sendBuffer[i];
		sendBuffer[i] = '\0';
	}
}

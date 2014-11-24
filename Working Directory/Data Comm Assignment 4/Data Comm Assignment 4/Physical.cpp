/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Physical.cpp -
--
-- PROGRAM: Irregardless
--
-- FUNCTIONS:
--		DWORD WINAPI startComms(LPVOID data);
--
-- DATE: November 19, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/

#include "Physical.h"

char sendSync;
bool sending = false;
bool receiving = false;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startComms
--
-- DATE: November 17, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- INTERFACE: DWORD WINAPI startComms(LPVOID data)
--
-- RETURNS: DWORD status. The return status of the thread
--
-- NOTES:
-- This thread will begin the idle loop of the Grapefruit protocol and begin waiting for an ENQ
-- and checking the buffer for data.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI startComms(LPVOID data)
{
	HANDLE hRxThrd;
	hRxThrd = CreateThread(NULL, 0, startRx, NULL, 0, NULL);
	if (!hRxThrd)
	{
		MessageBox(NULL, TEXT("Error creating Rx thread."), TEXT("Error"), MB_ICONWARNING | MB_OK);
		CloseHandle(hRxThrd);
		hRxThrd = NULL;
		PostQuitMessage(0);
	}

	for(;;)
	{
		if(!receiving)
		{
			if(isBufferNotEmpty() && isResetTimerUp())
			{
				sending = true;
				sendSync = SYN1;

				sendControlChar(ENQ);
				waitForEnqResponse();
			}
		}
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: waitForEnqResponse
--
-- DATE: November 24, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- INTERFACE: void waitForEnqResponse(void)
--
-- RETURNS: void.
--
-- NOTES:
-- Waits for a response from the receiving end and posts the buffer data if it receives one.
----------------------------------------------------------------------------------------------------------------------*/
void waitForEnqResponse()
{
	if(waitForControlChar(ACK))
	{
		sendData();
	}

	sending = false;
	setTimer(getResetTime(&timeouts));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendData
--
-- DATE: November 24, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- INTERFACE: void sendData(void)
--
-- RETURNS: void.
--
-- NOTES:
-- Keeps trying to send until the maximum number of misses or sends have been hit, or the buffer is empty.
----------------------------------------------------------------------------------------------------------------------*/
void sendData()
{
	int misses = 0;
	int sent = 0;

	unsigned char packet[1024];

	while(isBufferNotEmpty() && misses < MAX_MISS && sent < MAX_SEND)
	{
		constructPacket(packet, MAX_SEND);
		int dataSize = min(buffersize, 1018);
		sendPacket(packet);
		char response = receiveControlChar();
		if(response == ACK)
		{
			Statistics.incACKReceived();
			Statistics.incGoodPacketSent();
			sent++;
			sendSync = (sendSync==SYN1)?SYN2:SYN1;
			popFromBuffer(dataSize);
		}
		else
		{
			misses++;
			Statistics.incNAKReceived();
			Statistics.incLostPacketSent();
		}
	}
	
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendPacket
--
-- DATE: November 18, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Thomas Tallentire
--
-- PROGRAMMER: Thomas Tallentire
--
-- INTERFACE: BOOL sendPacket(unsigned char* packet)
--
-- RETURNS: BOOl, whether or not the packet was sent properly.
--
-- NOTES:
-- The function sends a given packet over the comm port.
----------------------------------------------------------------------------------------------------------------------*/
BOOL sendPacket(unsigned char* packet)
{
	return (WriteFile(hComm, &packet, PACKET_SIZE, NULL, &ol));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: receivePacket
--
-- DATE: November 18, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Thomas Tallentire
--
-- PROGRAMMER: Thomas Tallentire
--
-- INTERFACE: BOOL receivePacket(unsigned char* packet)
--
-- RETURNS: BOOl, whether or not the packet was received properly.
--
-- NOTES:
-- The function waits for a packet of size defined by the PACKET_SIZE 
-- constant.
----------------------------------------------------------------------------------------------------------------------*/
BOOL receivePacket(unsigned char* packet)
{
	DWORD dwEvent;
	HANDLE hData = packet;
	BOOL ret;

	dwEvent = WaitForMultipleObjects(PACKET_SIZE, &hData, FALSE, timeouts.timeoutSendAck);

	switch(dwEvent)
	{
		case (WAIT_OBJECT_0 + PACKET_SIZE) :
			ret = TRUE;
			break;
		case (WAIT_ABANDONED_0 + 1) :
			ret = FALSE;
			break;
		case (WAIT_TIMEOUT) :
			ret = FALSE;
			break;
		case (WAIT_FAILED) :
			ret = FALSE;
			break;
		default:
			printf("Wait error: %d\n", GetLastError()); 
	}
	return ret;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendControlChar
--
-- DATE: November 18, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Thomas Tallentire
--
-- PROGRAMMER: Thomas Tallentire
--
-- INTERFACE: BOOL sendControlChar(char cChar)
--
-- RETURNS: BOOl, whether or not the control character was sent properly.
--
-- NOTES:
-- The function attempts to send a control character over the comm port.
----------------------------------------------------------------------------------------------------------------------*/
BOOL sendControlChar(char cChar)
{
	return (WriteFile(hComm, &cChar, 1, NULL, &ol));
}

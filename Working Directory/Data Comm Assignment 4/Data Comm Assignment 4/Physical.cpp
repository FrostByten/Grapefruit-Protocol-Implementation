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
char rxSync;
bool sending = false;
bool receiving = false;
bool waitForReset = false;
Statistics *stats;

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
	stats = Statistics::getInstance();

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
			if(isBufferNotEmpty())
			{
				sending = true;
				sendSync = SYN1;

				sendControlChar(ENQ);
				printDebugString("TEST");
				stats->incENQSent();
				waitForEnqResponse();
			}
			sending = false;
			if (waitForReset)
			{
				Sleep(getResetTime(&timeouts));
				waitForReset = false;
			}
		}
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startRx
--
-- DATE: November 17, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- INTERFACE: DWORD WINAPI startRx(LPVOID data)
--
-- RETURNS: DWORD status. The return status of the thread
--
-- NOTES:
-- This thread will begin the idle loop of the Grapefruit protocol and begin waiting for an ENQ.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI startRx(LPVOID data)
{
	for(;;)
	{
		if(!sending)
		{
			if (receiveControlChar(ENQ, timeouts.timeoutSendAck))
			{
				receiving = true;
				rxSync = SYN1;
				stats->incENQReceived();

				sendControlChar(ACK);
				stats->incACKSent();
				waitForAckResponse();
			}
			receiving = false;
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
	if (receiveControlChar(ACK, timeouts.timeoutSendEnq))
		sendData();

	sending = false;
	waitForReset = true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: waitForAckResponse
--
-- DATE: November 24, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- INTERFACE: void waitForAckResponse(void)
--
-- RETURNS: void.
--
-- NOTES:
-- Waits for a response from the sending end and begins receiving packets if it receives one.
----------------------------------------------------------------------------------------------------------------------*/
void waitForAckResponse()
{
	unsigned char pack[PACKET_SIZE];

	while(receiving)
	{
		if(!receivePacket(pack))
			receiving = false;
		else
		{
			stats->incGoodPacketReceived();
			pushPacketToDisplayBuffer(pack);
			rxSync = rxSync == SYN1 ? SYN2 : SYN1;
		}
	}
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
		//constructPacket(packet, MAX_SEND);
		int dataSize = min(1024, 1018);
		sendPacket(packet);
		char response = 'f'; //receiveControlChar();
		if(response == ACK)
		{
			stats->incACKReceived();
			stats->incGoodPacketSent();
			sent++;
			sendSync = (sendSync==SYN1)?SYN2:SYN1;
			popFromBuffer(dataSize);
		}
		else
		{
			misses++;
			stats->incNAKReceived();
			stats->incLostPacketSent();
		}
	}

	sending = false;
	waitForReset = true;
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
-- constant and validates it. If the timeout is hit, the function returns FALSE,
-- if the packet is invalid, 
----------------------------------------------------------------------------------------------------------------------*/
DWORD receivePacket(unsigned char* packet)
{
	BOOL readRet;
	HANDLE hData = packet;
	DWORD ret = SUCCESSFUL_PACKET;
	DWORD sizeRead = 0;
	DWORD lastRead = 0;
	LPCOMMTIMEOUTS lpCommTimeouts = new COMMTIMEOUTS();
	lpCommTimeouts->ReadTotalTimeoutMultiplier = 0;
	lpCommTimeouts->ReadTotalTimeoutConstant = (DWORD)timeouts.timeoutSendAck;

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Error setting comm mask:", "", MB_OK);
		return SYSTEM_ERROR;
	}

	if (!SetCommTimeouts(hComm, lpCommTimeouts))
	{
		MessageBox(NULL, "Error setting comm timeouts:", "", MB_OK);
		return SYSTEM_ERROR;
	}

	readRet = ReadFile(hComm, packet, PACKET_SIZE, &lastRead, &ol);
		
	if (GetLastError() != ERROR_IO_PENDING)
	{
		ret = SYSTEM_ERROR;
	}
	if (lastRead != PACKET_SIZE) {
		ret = SYSTEM_ERROR;
	}
	if (!GetCommTimeouts(hComm, lpCommTimeouts))
	{
		ret = SYSTEM_ERROR;
		printf("GetCommTimeouts error: %d\n", GetLastError());
	}

	if (ret != SUCCESSFUL_PACKET)
			return ret;

	if (readRet)
	{
		ret = SUCCESSFUL_PACKET;
		if (validatePacket((unsigned char*)packet))
		{
			ret = SUCCESSFUL_PACKET;
		} else
		{
			ret = INVALID_PACKET;
		}
	} else
	{
		ret = SYSTEM_ERROR;
		printf("Wait error: %d\n", GetLastError()); 
	}
	return ret;
}

BOOL validatePacket(unsigned char *packet)
{
	return TRUE;
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: receiveControlChar
--
-- DATE: November 18, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Thomas Tallentire
--
-- PROGRAMMER: Thomas Tallentire
--
-- INTERFACE: BOOL receiveControlChar(char cChar)
--
-- RETURNS: BOOl, whether or not the control character was received properly.
--
-- NOTES:
-- The function waits for a control character on the comm port and returns 
-- whether or not the read character matches the passed in character.
----------------------------------------------------------------------------------------------------------------------*/
BOOL receiveControlChar(char cChar, double waitTimeout)
{
	DWORD numRead;
	BOOL readRet;
	BOOL ret;
	char temp = ' ';
	HANDLE receiveChar = &temp;
	LPCOMMTIMEOUTS lpCommTimeouts = new COMMTIMEOUTS();
	lpCommTimeouts->ReadTotalTimeoutMultiplier = 0;
	lpCommTimeouts->ReadTotalTimeoutConstant = (DWORD)waitTimeout;

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Error setting comm mask:", "", MB_OK);
		return SYSTEM_ERROR;
	}

	if (!SetCommTimeouts(hComm, lpCommTimeouts))
	{
		MessageBox(NULL, "Error setting comm timeouts:", "", MB_OK);
		return SYSTEM_ERROR;
	}

	readRet = ReadFile(hComm, receiveChar, sizeof(char), &numRead, &ol);

	if (readRet)
		ret = TRUE;
	else
		ret = FALSE;
	return ret;
}

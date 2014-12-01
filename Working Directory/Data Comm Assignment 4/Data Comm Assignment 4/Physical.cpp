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
				syncSend = SYN1;

				sendControlChar(ENQ);
				//printDebugString("TEST");
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
			if (receiveENQ())
			{
				receiving = true;
				syncRx = SYN1;
				stats->incENQReceived();
				sendControlChar(ACK);
				stats->incACKSent();
				//waitForAckResponse();
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
			syncRx = syncRx == SYN1 ? SYN2 : SYN1;
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
	BOOL maxSent = FALSE;

	unsigned char packet[PACKET_SIZE];

	while(isBufferNotEmpty() && misses < MAX_MISS && sent < MAX_SEND)
	{
		if (sent == MAX_SEND - 1)
			maxSent = TRUE;
		else
			maxSent = FALSE;
		int dataSize = 0;//constructPacket(packet, maxSent);
		sendPacket(packet);
		char response = receiveGenControlChar(timeouts.timeoutSendPacket);
		if (response != NULL)
		{
			if(response == ACK)
			{
				stats->incACKReceived();
				stats->incGoodPacketSent();
				sent++;
				syncSend = (syncSend==SYN1)?SYN2:SYN1;
				popFromBuffer(dataSize);
			}
			else
			{
				misses++;
				stats->incNAKReceived();
				stats->incLostPacketSent();
			}
		}
		else
		{
			misses++;
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
		/*if (validatePacket(packet))
		{
			ret = SUCCESSFUL_PACKET;
		} else
		{
			ret = INVALID_PACKET;
		}*/
	} else
	{
		ret = SYSTEM_ERROR;
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
	/*if(!WriteFile(hComm, &cChar, 1, NULL, &ol))
	{
		DWORD err = GetLastError();
 		if (err == 0x3e5)
 		{
			GetOverlappedResult(hComm, &ol, NULL, TRUE);
		}
		return TRUE;
	}
	return TRUE;*/
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
-- INTERFACE: BOOL receiveControlChar(double waitTimeout)
--
-- RETURNS: BOOl, whether or not the control character was received properly.
--
-- NOTES:
-- The function waits for a control character on the comm port for a certain timeout.
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
	{
		if ((char)receiveChar == cChar)
			ret = TRUE;
		else
			ret = FALSE;
	}
	else
		ret = FALSE;
	return ret;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: receiveENQ
--
-- DATE: November 18, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Thomas Tallentire
--
-- PROGRAMMER: Thomas Tallentire
--
-- INTERFACE: BOOL receiveENQ()
--
-- RETURNS: BOOL, whether or not the ENQ was received properly.
--
-- NOTES:
-- The function waits for a control character on the comm port for a certain timeout.
----------------------------------------------------------------------------------------------------------------------*/
BOOL receiveENQ()
{
	DWORD numRead;
	DWORD dwCommEvent;
	BOOL ret = FALSE;
	char temp;
	LPVOID receiveChar = &temp;

	// Set the comm mask to receiving a char
	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Error setting comm mask:", "", MB_OK);
		return SYSTEM_ERROR;
	}
	
	// Wait for a char to appear
	if (WaitCommEvent(hComm, &dwCommEvent, &ol))
	{
		// Read a char from the file and check if it's ENQ
		if (!ReadFile(hComm, &temp, 1, &numRead, &ol))
		{
			DWORD err = GetLastError();
 			if (err == 0x3e5)
 			{
				GetOverlappedResult(hComm, &ol, &numRead, TRUE);
			}
		}
		if(temp == ENQ)
			return TRUE;
	}
	else
	{
		GetOverlappedResult(hComm, &ol, &numRead, TRUE);
		// Read a char from the file and check if it's ENQ
		if (!ReadFile(hComm, &temp, 1, &numRead, &ol))
		{
			DWORD err = GetLastError();
 			if (err == 0x3e5)
 			{
				GetOverlappedResult(hComm, &ol, &numRead, TRUE);
			}
		}
		if(temp == ENQ)
			return TRUE;
		else
			return FALSE;
	}

	return ret;
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
-- INTERFACE: BOOL receiveControlChar(double waitTimeout)
--
-- RETURNS: BOOl, whether or not the control character was received properly.
--
-- NOTES:
-- The function waits for a control character on the comm port for a certain timeout.
----------------------------------------------------------------------------------------------------------------------*/
char receiveGenControlChar(double waitTimeout)
{
	DWORD numRead;
	BOOL readRet;
	char temp = ' ';
	HANDLE receiveChar = &temp;
	LPCOMMTIMEOUTS lpCommTimeouts = new COMMTIMEOUTS();
	lpCommTimeouts->ReadTotalTimeoutMultiplier = 0;
	lpCommTimeouts->ReadTotalTimeoutConstant = (DWORD)waitTimeout;

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Error setting comm mask:", "", MB_OK);
	}

	if (!SetCommTimeouts(hComm, lpCommTimeouts))
	{
		MessageBox(NULL, "Error setting comm timeouts:", "", MB_OK);
	}

	readRet = ReadFile(hComm, receiveChar, sizeof(char), &numRead, &ol);

	return (char)receiveChar;
}

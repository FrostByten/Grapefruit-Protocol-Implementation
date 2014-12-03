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
#include <iomanip>

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
			syncSend = SYN1;
			if(isBufferNotEmpty())
			{
				TerminateThread(hRxThrd, 0);
				sending = true;

				sendControlChar(ENQ);
				stats->incENQSent();
				waitForEnqResponse();
				hRxThrd = CreateThread(NULL, 0, startRx, NULL, 0, NULL);
				if (!hRxThrd)
				{
					MessageBox(NULL, TEXT("Error creating Rx thread."), TEXT("Error"), MB_ICONWARNING | MB_OK);
					CloseHandle(hRxThrd);
					hRxThrd = NULL;
					PostQuitMessage(0);
				}
			}

			sending = false;
			
			if (waitForReset)
			{
				Sleep(getResetTime(&timeouts)+100);
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
	{
		stats->incACKReceived();
		sendData();
		return;
	}

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
		DWORD rec = receivePacket(pack);
		switch (rec)
		{
			case SUCCESSFUL_PACKET:
				sendControlChar(ACK);
				stats->incGoodPacketReceived();
				stats->incACKSent();
				pushPacketToDisplayBuffer(pack);
				syncRx = syncRx == SYN1 ? SYN2 : SYN1;
				if (pack[0] == EOT)
					receiving = false;
				break;
			case INVALID_PACKET:
				stats->incBadPacketReceived();
				stats->incNAKSent();
				sendControlChar(NAK);
				break;
			case TIMEOUT_PACKET:
				stats->incLostPacketReceived();
				receiving = false;
				break;
			default:
				exit(0);
				break;
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
		int dataSize = constructPacket(packet, maxSent);
		sendPacket(packet);
		char response = receiveGenControlChar(timeouts.timeoutSendPacket);
		if(response == ACK)
		{
			stats->incACKReceived();
			stats->incGoodPacketSent();
			sent++;
			popFromBuffer(dataSize);
		}
		else
		{
			misses++;
			if (response == NAK)
			{
				stats->incBadPacketSent();
				stats->incNAKReceived();
			}
			else
			{
				stats->incLostPacketSent();
			}
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
	DWORD blank;

	if (!WriteFile(hComm, packet, PACKET_SIZE, NULL, &ol))
	{
		GetOverlappedResult(hComm, &ol, &blank, TRUE);
	}

	return TRUE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: receivePacket
--
-- DATE: November 18, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Thomas Tallentire
--				Lewis Scott
--
-- PROGRAMMER: Thomas Tallentire
--				Lewis Scott
--
-- INTERFACE: BOOL receivePacket(unsigned char* packet)
--
-- RETURNS: BOOL, whether or not the packet was received properly.
--
-- NOTES:
-- The function waits for a packet of size defined by the PACKET_SIZE 
-- constant and validates it. If the timeout is hit, the function returns FALSE,
-- if the packet is invalid, 
----------------------------------------------------------------------------------------------------------------------*/
DWORD receivePacket(unsigned char* packet)
{
	OutputDebugString("Enter receivePacket\n");
	int totRead = 0;
	DWORD read = 0;
	DWORD dwCommEvent;
	DWORD dwMaskEvent;
	DWORD blank;
	SYSTEMTIME start;
	SYSTEMTIME now;
	LPCOMMTIMEOUTS lpCommTimeouts = new COMMTIMEOUTS();
	lpCommTimeouts->ReadTotalTimeoutMultiplier = 0;
	lpCommTimeouts->ReadTotalTimeoutConstant = (DWORD)timeouts.timeoutSendAck;

	PurgeComm(hComm, PURGE_RXCLEAR);
	PurgeComm(hComm, PURGE_TXCLEAR);
	PurgeComm(hComm, PURGE_TXABORT);
	PurgeComm(hComm, PURGE_RXABORT);

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Error setting comm mask:", "", MB_OK);
	}

	/*if (!SetCommTimeouts(hComm, lpCommTimeouts))
	{
		MessageBox(NULL, "Error setting comm timeouts:", "", MB_OK);
		return SYSTEM_ERROR;
	}*/

	GetSystemTime(&start);
	GetSystemTime(&now);

	while (totRead < PACKET_SIZE)
	{
		OutputDebugString("Packet not full\n");
		if (!WaitCommEvent(hComm, &dwCommEvent, &ol))
		{
			DWORD err = GetLastError();
			if (err == 0x3e5)
			{
				GetOverlappedResult(hComm, &ol, &blank, TRUE);
			}
			else
			{
				OutputDebugString("Not IO pending");
				return TIMEOUT_PACKET;
			}
		}
		if (dwCommEvent != EV_RXCHAR)
			return SYSTEM_ERROR;
		if (!ReadFile(hComm, packet + totRead, PACKET_SIZE, &read, &ol))
		{
			GetOverlappedResult(hComm, &ol, &read, TRUE);
		}
		totRead += read;

		GetSystemTime(&now);
		if (((now.wMilliseconds - start.wMilliseconds) + (now.wSecond - start.wSecond) * 1000)> timeouts.timeoutSendAck)
		{
			OutputDebugString("mil - mil\n");
			return TIMEOUT_PACKET;
		}
	}

	if (validatePacket(packet))
	{
		return SUCCESSFUL_PACKET;
	} 
	else
	{
		return INVALID_PACKET;
	}
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
	DWORD temp;

	if(!WriteFile(hComm, &cChar, 1, &temp, &ol))
	{
		DWORD err = GetLastError();
 		if (err == 0x3e5)
 		{
			GetOverlappedResult(hComm, &ol, &temp, TRUE);
		}
		return TRUE;
	}
	return TRUE;
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
	BOOL readRet = FALSE;
	char temp = 'X';
	DWORD dwCommEvent;
	DWORD dwMaskEvent;
	SYSTEMTIME start;
	SYSTEMTIME now;
	LPCOMMTIMEOUTS lpCommTimeouts = new COMMTIMEOUTS();
	lpCommTimeouts->ReadTotalTimeoutMultiplier = 0;
	lpCommTimeouts->ReadTotalTimeoutConstant = (DWORD)waitTimeout;

	PurgeComm(hComm, PURGE_RXCLEAR);
	PurgeComm(hComm, PURGE_TXCLEAR);
	PurgeComm(hComm, PURGE_TXABORT);
	PurgeComm(hComm, PURGE_RXABORT);

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Error setting comm mask:", "", MB_OK);
	}

	/*if (!SetCommTimeouts(hComm, lpCommTimeouts))
	{
		MessageBox(NULL, "Error setting comm timeouts:", "", MB_OK);
		return SYSTEM_ERROR;
	}*/

	GetSystemTime(&start);
	if (WaitCommEvent(hComm, &dwCommEvent, &ol))
	{
		if (!ReadFile(hComm, &temp, 1, &numRead, &ol))
		{
			GetOverlappedResult(hComm, &ol, &numRead, TRUE);
		}
	}
	else
	{
		DWORD err = GetLastError();
 		if (err == ERROR_IO_PENDING)
		{
			GetOverlappedResult(hComm, &ol, &numRead, TRUE);
			if (dwCommEvent != EV_RXCHAR)
				return FALSE;
			if (!ReadFile(hComm, &temp, 1, &numRead, &ol))
			{
				GetOverlappedResult(hComm, &ol, &numRead, TRUE);
			}
		}
	}

	GetSystemTime(&now);
	if (((now.wMilliseconds - start.wMilliseconds) + (now.wSecond - start.wSecond) * 1000)> timeouts.timeoutSendAck)
	{
		return TIMEOUT_PACKET;
	}

	if(temp == cChar)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
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
	DWORD dwMaskEvent;
	BOOL ret = FALSE;
	char temp;
	LPVOID receiveChar = &temp;


	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Error setting comm mask:", "", MB_OK);
	}
	
	// Wait for a char to appear
	if (WaitCommEvent(hComm, &dwCommEvent, &ol))
	{
		if (dwCommEvent != EV_RXCHAR)
			return FALSE;
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
	DWORD dwCommEvent;
	DWORD dwMaskEvent;
	char receivechar[2] = { '\0', '\0' };
	SYSTEMTIME start;
	SYSTEMTIME now;
	LPCOMMTIMEOUTS lpCommTimeouts = new COMMTIMEOUTS();
	lpCommTimeouts->ReadTotalTimeoutMultiplier = 0;
	lpCommTimeouts->ReadTotalTimeoutConstant = (DWORD)waitTimeout;

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Error setting comm mask:", "", MB_OK);
	}

	GetSystemTime(&start);
	if (!WaitCommEvent(hComm, &dwCommEvent, &ol))
	{	
		DWORD err = GetLastError();
 		if (err == 0x3e5)
		{
			GetOverlappedResult(hComm, &ol, &numRead, TRUE);
		}
		else
		{
			OutputDebugString("timed out receiving general control char");
			return '\0';
		}
	}
	if (dwCommEvent != EV_RXCHAR)
		return NULL;
		
	
	if (!ReadFile(hComm, receivechar, sizeof(char), &numRead, &ol))
	{
		GetOverlappedResult(hComm, &ol, &numRead, TRUE);
	}

	GetSystemTime(&now);
	if (((now.wMilliseconds - start.wMilliseconds) + (now.wSecond - start.wSecond) * 1000)> timeouts.timeoutSendAck)
	{
		return NULL;
	}

	return receivechar[0];
}

/*BOOL receiveAfterENQ(double waitTimeout)
{
	DWORD numRead;
	DWORD dwCommEvent;
	DWORD dwMaskEvent;
	char receivechar[2] = { '\0', '\0' };
	SYSTEMTIME start;
	SYSTEMTIME now;

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Error setting comm mask:", "", MB_OK);
	}
	OutputDebugString("Starting to look for ACK");
	GetSystemTime(&start);
	while (!WaitCommEvent(hComm, &dwCommEvent, &ol))
	{
		GetSystemTime(&now);
		if (((now.wMilliseconds - start.wMilliseconds) + (now.wSecond - start.wSecond) * 1000) > timeouts.timeoutSendAck)
		{
			return FALSE;
		}
	}
	OutputDebugString("Done looking");
	if (!ReadFile(hComm, receivechar, sizeof(char), &numRead, &ol))
	{
		return FALSE;
	}
	OutputDebugString("Read successful");
	if (receivechar[0] == ACK)
		return TRUE;
	else
		return FALSE;
}*/
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
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI startComms(LPVOID data)
{
	DWORD read;
	char recieved = '\0';
	DWORD eventmask = EV_RXCHAR;
	
	for(;;)
	{
		read = 0;
		WaitCommEvent(hComm, &eventmask, &ol);

		if (ReadFile(hComm, &recieved, 1, &read, &ol))
		{
			char disp[2] = { recieved, '\0' };
			if (read && recieved != '\0')
				MessageBox(NULL, disp, TEXT("Such wow!"), MB_OK);
		}
		else
		{
			DWORD err = GetLastError();
			if (err == 0x3e5)
			{
				GetOverlappedResult(hComm, &ol, &read, TRUE);
				char disp[2] = { recieved, '\0' };
				if (read && recieved != '\0')
					MessageBox(NULL, disp, TEXT("Such wow!"), MB_OK);
			}
		}
		recieved = '\0';
	}

	return 0;
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

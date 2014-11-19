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
	char recieved;
	DWORD eventmask = EV_RXCHAR;
	
	for(;;)
	{
		WaitCommEvent(hComm, &eventmask, &ol);

		if (ReadFile(hComm, &recieved, 1, &read, &ol))
		{
			char disp[2] = { recieved, '\0' };
			if (read)
				MessageBox(NULL, disp, TEXT("Such wow!"), MB_OK);
		}
		else
		{
			DWORD err = GetLastError();
			if (err == 0x3e5)
			{
				DWORD trans;
				GetOverlappedResult(hComm, &ol, &trans, TRUE);
				char disp[2] = { recieved, '\0' };
				if (trans && read)
					MessageBox(NULL, disp, TEXT("Such wow!"), MB_OK);
			}
		}
	}

	return 0;
}

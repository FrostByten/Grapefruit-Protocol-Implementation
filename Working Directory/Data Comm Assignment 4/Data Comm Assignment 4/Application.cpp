/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Application.cpp -
--
-- PROGRAM: Irregardless
--
-- FUNCTIONS:
--		int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance,
--					  LPSTR lspszCmdParam, int nCmdShow);
--		LRESULT CALLBACK WndProc (HWND hwnd, UINT Message,
--					WPARAM wParam, LPARAM lParam);
--		void clearString(char*);
--		void printDebugString(char* str);
--		
--
-- DATE: November 15, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--			 Lewis Scott
--
-- PROGRAMMER: Christofer Klassen
--			   Lewis Scott
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/

#define STRICT
#define _CRT_SECURE_NO_WARNINGS

//#define CONNECT_ON_START
#define RANDOMIZE_SEED

#pragma warning (disable: 4096)
#pragma warning (disable: 4018)
#pragma warning (disable: 4244)


#include <stdio.h>
#include "Menu.h"
#include "Physical.h"

char Name[] = "Irregardless Peer-to-Peer via Grapefruit";
char printText[2048] = "TEST";	//output buffer
char sendBuffer[SEND_BUF_SIZE]; //input buffer

unsigned char syncSend;
unsigned char syncRx;

int X = 0, Y = 0; // Current coordinates
int analyticsDivider = 400;

stringstream analytics;

// Timeouts
Timeouts timeouts;

HANDLE hThrd;
HANDLE hComm;
DCB dcb;
HMENU hMenu;
HWND hwnd;
HDC hdc;
PAINTSTRUCT paintstruct;

WNDCLASSEX Wcl;
OVERLAPPED ol = { 0 };

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: November 17, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--			 Lewis Scott
--
-- PROGRAMMER: Christofer Klassen
--			   Lewis Scott
--
-- INTERFACE: int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance,
--					  LPSTR lspszCmdParam, int nCmdShow);
--
-- RETURNS: int - the execution code of the program
--
-- NOTES:
-- This function is the main function of the program. It establishes a link
-- to the communications port and initializes protocol variables, as well as
-- starting the WndProc message loop.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance,
 						  LPSTR lspszCmdParam, int nCmdShow)
{
	#ifdef RANDOMIZE_SEED
		srand(time(NULL));
	#endif

	BuildCommDCB(TEXT("96,N,8,1"), &dcb);

	#ifdef CONNECT_ON_START
		if ((hComm = CreateFile(TEXT("COM1"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 
			FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE || !SetCommState(hComm, &dcb))
		{
			MessageBox(NULL, TEXT("Error connecting to modem, exiting..."), TEXT("Error"), MB_OK);
			PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR);
			CloseHandle(hComm);
			hComm = NULL;
			PostQuitMessage(0);
		}

		ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		hThrd = CreateThread(NULL, 0, startComms, NULL, 0, NULL);
		if (!hThrd)
		{
			MessageBox(NULL, TEXT("Error creating communications thread."), TEXT("Error"), MB_ICONWARNING | MB_OK);
			CloseHandle(hThrd);
			hThrd = NULL;
			PostQuitMessage(0);
		}
	#endif

	// Calculate timeouts
	calculateTimeouts(&timeouts);

	// Create initial analytic values
	updateAnalytics();

	MSG Msg;

	Wcl.cbSize = sizeof (WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style
	
	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH); //gray background
	Wcl.lpszClassName = Name;
	
	Wcl.lpszMenuName = TEXT("MENU"); // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0; 
	
	if (!RegisterClassEx (&Wcl))
	{
		return 0;
	}

	hwnd = CreateWindow (Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
   							600, 400, NULL, NULL, hInst, NULL);
	ShowWindow (hwnd, nCmdShow);
	UpdateWindow (hwnd);

	while (GetMessage (&Msg, NULL, 0, 0))
	{
   		TranslateMessage (&Msg);
		DispatchMessage (&Msg);
	}

	return Msg.wParam;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WndProc
--
-- DATE: November 17, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--			 Lewis Scott
--
-- PROGRAMMER: Christofer Klassen
--			   Lewis Scott
--
-- INTERFACE: LRESULT CALLBACK WndProc (HWND hwnd, UINT Message,
--					WPARAM wParam, LPARAM lParam);
--
-- RETURNS: LRESULT
--
-- NOTES:
-- This function handles all messages received by the program.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc (HWND hwnd, UINT Message,
                          WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_COMMAND:

			switch (LOWORD (wParam))
			{
				case IDM_HELP:
				{
					const char* HELP_TEXT = "Irregardless is a communications program designed to "
						"implement and display the functionality of the Grapefruit "
						"Protocol for peer - to - peer wired and wireless communication. "
						"\n\n"
						"Irregardless contains functionality for transmitting single "
						"characters and full text files across wired and wireless "
						"environments, and is capable of tracking protocol statistics "
						"for debugging and analytical purposes.";
					MessageBox(hwnd, HELP_TEXT, TEXT("Help"), MB_OK | MB_ICONINFORMATION);
					break;
				}
				case IDM_QUIT:
					PostQuitMessage(0);
					break;
				case IDM_SENDTEXTFILE:
					// TODO: Add a text file to the buffer
					break;
			}
			break;
			
		case WM_CHAR:
			// TODO: Add a character to the send buffer
			if(wParam >= 0x20 && wParam <= 0x7E) 
				WriteFile(hComm, &wParam, 1, NULL, &ol);
			break;

		case WM_PAINT:		// Process a repaint message
			// Update the divider location
			RECT rect;
			GetWindowRect(hwnd, &rect);
			analyticsDivider = rect.right - rect.left - ANALYTICS_WIDTH;

			hdc = BeginPaint (hwnd, &paintstruct); // Acquire DC
			TextOut (hdc, 0, 0, printText, strlen (printText)); // output character
		
			EndPaint (hwnd, &paintstruct); // Release DC

			// Print analytics
			updateAnalytics();

			break;

		case WM_DESTROY:
      		PostQuitMessage(0);
			break;

		default:
			return DefWindowProc (hwnd, Message, wParam, lParam);
	}
	return 0;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: refreshScreen
--
-- DATE: November 27, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--
-- PROGRAMMER: Christofer Klassen
--
-- INTERFACE: void refreshScreen();
--
-- RETURNS: void
--
-- NOTES:
-- This function refreshes the screen, forcing a WM_PAINT call.
----------------------------------------------------------------------------------------------------------------------*/
void refreshScreen()
{
	InvalidateRect(hwnd, NULL, true);

	return;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateAnalytics
--
-- DATE: November 24, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--
-- PROGRAMMER: Christofer Klassen
--
-- INTERFACE: void updateAnalytics();
--
-- RETURNS: void
--
-- NOTES:
-- This function re-pulls analytical information from the static Statistics object
-- and updates the text on-screen.
----------------------------------------------------------------------------------------------------------------------*/
void updateAnalytics()
{
	Statistics* stats = Statistics::getInstance();

	// Empty the string
	analytics.clear();

	// Add analytic counters
	analytics << "ACK Received: " << stats->getACKReceived() << "\n";
	analytics << "NAK Received: " << stats->getNAKReceived() << "\n";
	analytics << "ACK Sent: " << stats->getACKSent() << "\n";
	analytics << "NAK Sent: " << stats->getNAKSent() << "\n";
	analytics << "ENQ Received: " << stats->getENQReceived() << "\n";
	analytics << "ENQ Sent: " << stats->getENQSent() << "\n";
	analytics << "Bad Packet Received: " << stats->getBadPacketReceived() << "\n";
	analytics << "Bad Packet Sent: " << stats->getBadPacketSent() << "\n";

	drawAnalytics();

	return;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: drawAnalytics
--
-- DATE: November 24, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--
-- PROGRAMMER: Christofer Klassen
--
-- INTERFACE: void drawAnalytics();
--
-- RETURNS: void
--
-- NOTES:
-- This function displays the analytics data to the screen.
----------------------------------------------------------------------------------------------------------------------*/
void drawAnalytics()
{
	int y = 0;
	string analyticsSection;

	// Acquire DC
	hdc = GetDC(hwnd);

	// Loop through and display each analytic
	do
	{
		analyticsSection.clear();
		getline(analytics, analyticsSection);
		TextOut(hdc, analyticsDivider, y, analyticsSection.c_str(), analyticsSection.size());
		
		y += 16;

	} while (analyticsSection.size() > 0);

	// Release DC
	ReleaseDC(hwnd, hdc); 

	return;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: saveAnalytics
--
-- DATE: November 26, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--
-- PROGRAMMER: Christofer Klassen
--
-- INTERFACE: void saveAnalytics();
--
-- RETURNS: void
--
-- NOTES:
-- This function saves all analytics to a file.
----------------------------------------------------------------------------------------------------------------------*/
void saveAnalytics()
{
	Statistics* stats = Statistics::getInstance();

	ofstream file;
	file.open("Protocol Analytics.txt");

	file << "ACK Received: " << stats->getACKReceived() << "\n";
	file << "NAK Received: " << stats->getNAKReceived() << "\n";
	file << "ACK Sent: " << stats->getACKSent() << "\n";
	file << "NAK Sent: " << stats->getNAKSent() << "\n";
	file << "ENQ Received: " << stats->getENQReceived() << "\n";
	file << "ENQ Sent: " << stats->getENQSent() << "\n";
	file << "Bad Packet Received: " << stats->getBadPacketReceived() << "\n";
	file << "Bad Packet Sent: " << stats->getBadPacketSent() << "\n";

	file.close();

	return;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: clearString
--
-- DATE: September 27, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--
-- PROGRAMMER: Christofer Klassen
--
-- INTERFACE: void clearString(char*)
--
-- RETURNS: void
--
-- NOTES:
-- This function empties the string buffer that contains the text that is printed
-- to the screen from the port.
----------------------------------------------------------------------------------------------------------------------*/
void clearString(char* str)
{
	for (int i = 0; i < strlen(str); i++)
	{
		str[i] = '\0';
	}

	X = 0;
	Y = 0;

	return;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: printDebugString
--
-- DATE: November 19, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--
-- PROGRAMMER: Christofer Klassen
--
-- INTERFACE: void printDebugString(char* str);
--
-- RETURNS: void
--
-- NOTES:
-- This function displays a Message Box with a certain string.
----------------------------------------------------------------------------------------------------------------------*/
void printDebugString(char* str)
{
	MessageBox(NULL, str, "Testing", MB_OK);
	return;
}

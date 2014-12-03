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
--		void addToTotalMessage();
--		void displayReceived();
--
--
-- DATE: November 15, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--			 Lewis Scott
--			 Jeff Bayntun
--
-- PROGRAMMER: Christofer Klassen
--			   Lewis Scott
--			   Jeff Bayntun
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/

#define STRICT
#define _CRT_SECURE_NO_WARNINGS

#define CONNECT_ON_START
#define RANDOMIZE_SEED

#pragma warning (disable: 4096)
#pragma warning (disable: 4018)
#pragma warning (disable: 4244)


#include <stdio.h>
#include "Menu.h"
#include "Physical.h"

char Name[] = "Irregardless Peer-to-Peer via Grapefruit";
char printText[2048];	//output buffer
char sendBuffer[SEND_BUF_SIZE]; //input buffer

unsigned char syncSend;
unsigned char syncRx;

FAR WNDPROC DefEditProc;

std::vector<string> totalMessage;

int X = 0, Y = 0; // Current coordinates
int analyticsDivider = 400;
int oldDivider = 400;

// Edit text positioning
int editButtonWidth = 0;

int editTextWidth = 0;
int editTextHeight = 0;


stringstream analytics;

// Timeouts
Timeouts timeouts;

HANDLE hThrd;
HANDLE hComm;
HWND hWndButton;
HWND hEdit;
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
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
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

	// set up crc table
	crcInit();
	// Calculate timeouts
	calculateTimeouts(&timeouts);

	stringstream msg;
	msg << timeouts.timeoutSendEnq;

	// Create initial analytic values
	updateAnalytics();

	MSG Msg;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //gray background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = TEXT("MENU"); // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
	{
		return 0;
	}

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		600, 400, NULL, NULL, hInst, NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
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
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
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
			closePort(hComm);
			PostQuitMessage(0);
			break;
		case IDM_SENDTEXTFILE:
			addTextFile();
			break;
		case IDM_EXPORTANALYTICS:
			saveAnalytics();
			break;
		case IDC_SEND_BTN:
			fillSendBuffer();
			break;
		case IDM_CLEARDATA:
			clearData();
			break;
		}
		break;

	case WM_CHAR:
		break;

	case WM_PAINT:		// Process a repaint message
		hdc = BeginPaint(hwnd, &paintstruct); // Acquire DC

		EndPaint(hwnd, &paintstruct); // Release DC

		// Print analytics
		updateAnalytics();

		// Print received data
		displayReceived();

		break;

	case WM_SIZE:
		updateWrapLength();
		refreshScreen();
		break;

	case WM_CREATE:
		hWndButton = CreateWindowEx(NULL,
			"BUTTON",
			"Send",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			470, 317, 114, 25, hwnd, (HMENU)IDC_SEND_BTN,
			GetModuleHandle(NULL), NULL);

		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE,
			"EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
			0, 317, 470, 25, hwnd, (HMENU)IDC_EDIT_TXT,
			GetModuleHandle(NULL), NULL);

		SendMessage(hEdit, EM_SETLIMITTEXT, 1024, '\0');

		DefEditProc = (WNDPROC)GetWindowLong(hEdit, GWL_WNDPROC);
		SetWindowLong(hEdit, GWL_WNDPROC, (long)EditTxtProc);
		SetFocus(hEdit);
		break;

	case WM_DESTROY:
#ifdef CONNECT_ON_START
		closePort(hComm);
#endif
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: EditTxtProc
--
-- DATE: November 29, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- INTERFACE: LRESULT CALLBACK EditTxtProc(HWND hDlg,
--				UINT message, WPARAM wParam, LPARAM lParam)
--
-- RETURNS: LRESULT
--
-- NOTES:
-- This function handles all messages received by the edit control.
-- If the user hits the enter key while the edit control is in focus,
-- it calls fillSendBuffer.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK EditTxtProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CHAR:
		if (wParam == VK_RETURN)
		{
			fillSendBuffer();
			return(0);
		}
		else if (wParam == VK_ESCAPE)
		{
			int ndx = 0;
			for (int i = 0; i < 10; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					printText[ndx] = 'A';
					ndx++;
				}
				printText[ndx] = ' ';
				ndx++;
			}

			displayReceived();
		}
		else return((LRESULT)CallWindowProc((WNDPROC)DefEditProc, hDlg, message, wParam, lParam));
		break;
	default:
		return((LRESULT)CallWindowProc((WNDPROC)DefEditProc, hDlg, message, wParam, lParam));
		break;
	}
	return(0);
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addTextFile
--
-- DATE: December 1, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Christofer Klassen
--
-- PROGRAMMER: Christofer Klassen
--
-- INTERFACE: void addTextFile();
--
-- RETURNS: void
--
-- NOTES:
-- This function adds a text file to the send buffer.
----------------------------------------------------------------------------------------------------------------------*/
void addTextFile()
{
	ifstream f;

	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];

	// Create the openfilename struct settings
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.hwndOwner = hwnd;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = TEXT("All files(*.*)\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Prompt the user to select a file
	if (GetOpenFileName(&ofn))
	{
		f.open(ofn.lpstrFile);
	}

	// Check if the file is open
	if (f.is_open())
	{
		string str;

		// Add the contents of the text file to the send buffer
		do
		{
			str.clear();

			// Read in a line from the file
			getline(f, str);

			if (str.length() > 0)
			{
				char *textbuff = (char*)str.c_str();
				bool done = FALSE;

				// Add the line to the send buffer
				for (int i = 0; !done && i < (SEND_BUF_SIZE - 1024); i++)
				{
					if (sendBuffer[i] == '\0')
					{
						for (int j = 0; !done && j < 1024; j++)
						{
							if (textbuff[j] == '\0')
							{
								sendBuffer[i] = '\n';
								done = TRUE;
							}
							else
							{
								sendBuffer[i] = textbuff[j];
								i++;
							}
						}
					}
				}
			}
		} while (!f.eof());

		printDebugString(sendBuffer);

		f.close();


	}
	else
	{
		printDebugString("File not found.");
	}

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
	analytics << "Good Packet Received: " << stats->getGoodPacketReceived() << "\n";
	analytics << "Good Packet Sent: " << stats->getGoodPacketSent() << "\n";
	analytics << "Bad Packet Received: " << stats->getBadPacketReceived() << "\n";
	analytics << "Bad Packet Sent: " << stats->getBadPacketSent() << "\n";
	analytics << "Lost Packet Sent: " << stats->getLostPacketSent() << "\n";
	analytics << "Lost Packet Received: " << stats->getLostPacketReceived() << "\n";
	analytics << "Percent Bad Received: " << stats->getReceivedBadPacketPercent() << "\n";

	drawAnalytics();
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
	file << "\n";
	file << "Bad Packet Received: " << stats->getBadPacketReceived() << "\n";
	file << "Bad Packet Sent: " << stats->getBadPacketSent() << "\n";
	file << "Good Packet Received: " << stats->getGoodPacketReceived() << "\n";
	file << "Good Packet Sent: " << stats->getGoodPacketSent() << "\n";
	file << "Lost Packet Received: " << stats->getLostPacketReceived() << "\n";
	file << "Lost Packet Sent: " << stats->getLostPacketSent() << "\n";
	file << "\n";
	file << "Bad Received Percent: " << stats->getReceivedBadPacketPercent() << "\n";
	file << "Good Received Percent: " << stats->getReceivedGoodPacketPercent() << "\n";
	file << "Bad Sent Percent: " << stats->getSentBadPacketPercent() << "\n";
	file << "Good Sent Percent: " << stats->getSentGoodPacketPercent() << "\n";
	file << "\n";
	file << "Average Packet Sent Size: " << stats->getAvgPacketSentSize() << "\n";
	file << "Average Packet Received Size: " << stats->getAvgPacketReceivedSize() << "\n";

	file.close();
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
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: fillSendBuffer
--
-- DATE: November 29, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Lewis Scott
--
-- PROGRAMMER: Lewis Scott
--
-- INTERFACE: void fillSendBuffer();
--
-- RETURNS: void
--
-- NOTES:
-- This function fills the send buffer with the contents of IDC_EDIT_TXT
----------------------------------------------------------------------------------------------------------------------*/
void fillSendBuffer()
{
	TCHAR textbuff[1024];
	GetWindowText(hEdit, textbuff, 1024);

	for (int i = 0; i < (SEND_BUF_SIZE - 1024); i++)
	{
		if (sendBuffer[i] == '\0')
		{
			for (int j = 0; j < 1024; j++)
			{
				if (textbuff[j] == '\0')
				{
					sendBuffer[i] = '\n';
					SendMessage(hEdit, WM_SETTEXT, 1, '\0');
					return;
				}
				else
				{
					sendBuffer[i] = textbuff[j];
					i++;
				}
			}
			SendMessage(hEdit, WM_SETTEXT, 1, '\0');
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: displayReceived
--
-- DATE: Dec 1, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: void displayReceived();
--
-- RETURNS: void
--
-- NOTES:
-- If there is new data in the display buffer, printText, it will be displayed to the screen
----------------------------------------------------------------------------------------------------------------------*/
void displayReceived()
{
	int width = analyticsDivider / 9;

	hdc = GetDC(hwnd);
	if (strlen(printText) != 0)
		addToTotalMessage();

	for (size_t i = 0; i < totalMessage.size(); i++)
	{
		TextOut(hdc, 0, i * 16, totalMessage[i].c_str(), totalMessage[i].size());
	}

	ReleaseDC(hwnd, hdc);
	//refreshScreen();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateWrapLength
--
-- DATE: Dec 1, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Chris Klassen
--
-- PROGRAMMER: Chris Klassen
--
-- INTERFACE: void updateWrapLength();
--
-- RETURNS: void
--
-- NOTES:
--		If the width of the screen has changed, re-organize the total message vector
----------------------------------------------------------------------------------------------------------------------*/
void updateWrapLength()
{
	RECT rect;
	GetClientRect(hwnd, &rect);

	//Update the size of the edit box
	editButtonWidth = 114;

	editTextWidth = rect.right - rect.left - editButtonWidth;
	editTextHeight = 25;

	//MoveWindow(hEdit, 0, 0, 30, 30, 1);
	MoveWindow(hEdit, 0, rect.top + (rect.bottom - rect.top) - 25, editTextWidth, editTextHeight, 1);
	ShowWindow(hEdit, SW_SHOWNORMAL);
	MoveWindow(hWndButton, editTextWidth, rect.top + (rect.bottom - rect.top) - 25, editButtonWidth, editTextHeight, 1);
	ShowWindow(hWndButton, SW_SHOWNORMAL);

	// Update the divider position
	analyticsDivider = rect.right - rect.left - ANALYTICS_WIDTH;

	// Make a copy of the vector
	string message;

	// Print the totalMessage pieces into one long string
	for (vector<string>::iterator it = totalMessage.begin(); it != totalMessage.end(); it++)
	{
		message += *it;
		message += ' ';
	}

	totalMessage.clear();

	// Re-order the strings within the vector based on the new size
	int width = analyticsDivider / 9;

	while (message.length() > width)
	{
		string temp = message.substr(0, width);
		int spaceLoc;
		if ((spaceLoc = temp.rfind(' ')) != string::npos)
		{
			totalMessage.push_back(temp.substr(0, spaceLoc));
			// erase message and space
			message = message.erase(0, spaceLoc + 1);
		}
		else
		{
			totalMessage.push_back(temp);
			message = message.erase(0, width);
		}
	}

	if (message.length() != 0)
	{
		totalMessage.push_back(message);
	}

	return;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addToTotalMessage
--
-- DATE: Dec 1, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: void addToTotalMessage();
--
-- RETURNS: void
--
-- NOTES:
-- Adds data from display buffer printText to vector totalMessage, which contains all the messages received so
-- far.  If there is spaces in text that is greater than one linesize, it will split the message at that space.
----------------------------------------------------------------------------------------------------------------------*/
void addToTotalMessage()
{
	//string test = "First, buying real estate without an adequate down payment means extreme leverage, which brings added risk. No, houses do not go up forever. Therefore, buying with 5% down and twenty-times leverage means a lowly 10% correction in the market would wipe out all your savings. You’d owe more than the property’s worth. If you don’t think that’s possible, come back this time next year. We’ll talk.\n\nThen there’s the CMHC premium, which is north of 3% of the mortgaged amount for a minimal down payment. On a $500,000 mortgage, the cost is almost $16,000. Most people add this to their mortgage, so it gets amortized and effectively doubled over time. Your property then needs to appreciate an extra $30,000 just to break even. Bummer.\n\nIn short, a big down is good. This woman gets it. But what sense does it make to keep $200,000 in the tangerine guy’s shorts at 1.3%, when the inflation rate is 2.03% and all of the interest is taxable at your marginal rate? And what if house prices in your hood rise by 5% while you’re saving?\nRight. Fail. That money is actually shrinking due to inflation and taxes while you sit on it. Even at double or triple the interest rate, you’re falling further behind. And if you lock the cash into a five-year GIC to boost the return to 2.8% at some godforsaken online Lithuanian steelworkers’ benevolent Manitoba credit union, it’s not available to you should a great house deal materialize in 20 months from now.";
	string message(printText);

	//totalMessage.push_back("this is no1");
	//totalMessage.push_back("this is no2");
	//totalMessage.push_back("this is no3");

	int width = analyticsDivider / 9;

	while (message.length() > width)
	{
		string temp = message.substr(0, width);
		int spaceLoc;
		if ((spaceLoc = temp.rfind(' ')) != string::npos)
		{
			totalMessage.push_back(temp.substr(0, spaceLoc));
			// erase message and space
			message = message.erase(0, spaceLoc + 1);
		}
		else
		{
			totalMessage.push_back(temp);
			message = message.erase(0, width);
		}
	}

	if (message.length() != 0)
	{
		totalMessage.push_back(message);
	}
	printText[0] = '\0';
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: clearData
--
-- DATE: Dec 2, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Chris Klassen
--
-- PROGRAMMER: Chris Klassen
--
-- INTERFACE: void clearData();
--
-- RETURNS: void
--
-- NOTES:
--		Clears the received data and refreshes the screen.
----------------------------------------------------------------------------------------------------------------------*/
void clearData()
{
	totalMessage.clear();
	refreshScreen();

	return;
}

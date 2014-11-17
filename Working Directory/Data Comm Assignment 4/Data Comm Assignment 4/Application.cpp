#define STRICT
#define _CRT_SECURE_NO_WARNINGS

#define CONNECT_ON_START

#include <windows.h>
#include <stdio.h>
#include "Menu.h"
#include "Application.h"

HANDLE hComm;
DCB dcb;
HMENU hMenu;
HWND hwnd;
WNDCLASSEX Wcl;
HANDLE hThread;

char Name[] = "Irregardless Peer-to-Peer via Grapefruit";
char printText[255];	//output buffer
int X = 0, Y = 0; // Current coordinates

#pragma warning (disable: 4096)


int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance,
 						  LPSTR lspszCmdParam, int nCmdShow)
{
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
	#endif

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


LRESULT CALLBACK WndProc (HWND hwnd, UINT Message,
                          WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paintstruct;

	switch (Message)
	{
		case WM_COMMAND:

			switch (LOWORD (wParam))
			{
				case IDM_HELP:
					MessageBox (hwnd, "TEMP", TEXT("Help"), MB_OK | MB_ICONINFORMATION);
					break;
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
			break;

		case WM_PAINT:		// Process a repaint message
			hdc = BeginPaint (hwnd, &paintstruct); // Acquire DC
			TextOut (hdc, 0, 0, printText, strlen (printText)); // output character
			EndPaint (hwnd, &paintstruct); // Release DC
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
-- This void empties the string buffer that contains the text that is printed
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
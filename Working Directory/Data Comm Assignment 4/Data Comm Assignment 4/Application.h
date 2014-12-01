#ifndef _IRREGARDLESS
#define _IRREGARDLESS

#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <time.h>
#include <windows.h>

using std::srand;
using std::string;
using std::stringstream;
using std::ofstream;
using std::ifstream;

const int ANALYTICS_WIDTH = 200;
const int SEND_BUF_SIZE = 10240;

#define EOT 0x04
#define ETB 0x17
#define ETX 0x03
#define SYN1 0x12
#define SYN2 0x13
#define RVI 0x11
#define ENQ 0x05
#define ACK 0x06
#define NAK 0x15

void refreshScreen();
void updateAnalytics();
void drawAnalytics();
void saveAnalytics();
void clearString(char* str);
void printDebugString(char* str);
void fillSendBuffer();
void addTextFile();
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EditTxtProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

typedef struct Timeouts Timeouts;
struct Timeouts {
	double timeoutSendEnq;
	double timeoutSendAck;
	double timeoutSendPacket;
	int timeoutReset;
	int resetMin;
	int resetMax;
};

#endif
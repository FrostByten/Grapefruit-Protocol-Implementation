#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <time.h>

#ifndef _IRREGARDLESS
#define _IRREGARDLESS

using std::srand;
using std::string;
using std::stringstream;
using std::ofstream;

const int ANALYTICS_WIDTH = 200;

void refreshScreen();
void updateAnalytics();
void drawAnalytics();
void saveAnalytics();
void clearString(char* str);
void printDebugString(char* str);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

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
#include <string>
#include <sstream>
#include <cstdlib>
#include <time.h>

using std::srand;
using std::string;
using std::stringstream;

void clearString(char* str);
void printDebugString(char* str);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI startComms(LPVOID data);

typedef struct Timeouts Timeouts;
struct Timeouts {
	double timeoutSendEnq;
	double timeoutSendAck;
	double timeoutSendPacket;
	int timeoutReset;
	int resetMin;
	int resetMax;
};

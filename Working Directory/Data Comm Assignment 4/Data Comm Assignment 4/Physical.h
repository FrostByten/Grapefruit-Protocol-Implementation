#ifndef _PHYSICAL
#define _PHYSICAL

#include "DataLink.h"

#define PACKET_SIZE 1024
#define SYSTEM_ERROR 3
#define INVALID_PACKET 2
#define TIMEOUT_PACKET 1
#define SUCCESSFUL_PACKET 0

extern HANDLE hComm;
extern OVERLAPPED ol;
extern Timeouts timeouts;

DWORD WINAPI startComms(LPVOID data);
DWORD WINAPI startRx(LPVOID data);
BOOL sendPacket(unsigned char* packet);
DWORD receivePacket(unsigned char* packet);
BOOL sendControlChar(char cChar);
BOOL receiveControlChar(char cChar, double waitTimeout);
char receiveGenControlChar(double waitTimeout);
void waitForEnqResponse();
void waitForAckResponse();
void sendData();


#endif
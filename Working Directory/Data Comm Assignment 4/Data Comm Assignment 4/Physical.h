#ifndef _PHYSICAL
#define _PHYSICAL

#include <Windows.h>
#include "Session.h"

#define EOT 0x04
#define ETB 0x17
#define ETX 0x03
#define SYN1 0x12
#define SYN2 0x13
#define RVI 0x11
#define ENQ 0x05
#define ACK 0x06
#define NAK 0x15

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
BOOL validatePacket(unsigned char *packet);
BOOL sendControlChar(char cChar);
BOOL receiveControlChar(char cChar, double waitTimeout);
void waitForEnqResponse();
void waitForAckResponse();
void sendData();


#endif
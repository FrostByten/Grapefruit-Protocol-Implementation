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

extern HANDLE hComm;
extern OVERLAPPED ol;
extern Timeouts timeouts;

DWORD WINAPI startComms(LPVOID data);

#endif
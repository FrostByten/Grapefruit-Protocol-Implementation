#ifndef _DATALINK
#define _DATALINK

#include "Session.h"
#include "crc.h"

extern std::string buffer;
extern unsigned char syncSend;
extern unsigned char syncRx;


#define MAX_DATA 1018
#define MAX_SENDS 5


void constructPacket( unsigned char* packet, size_t maxSends ); 
void readPacket( unsigned char* packet );
void setCRC( unsigned char* packet );
std::string trimResponse( unsigned char* response );
bool validatePacket( unsigned char* response );


#endif
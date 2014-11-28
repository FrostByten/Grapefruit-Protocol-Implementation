#ifndef _DATALINK_
#define _DATALINK_

#include "Physical.h"
#include "crc.h"

#include <iostream>
#include <string>

std::string buffer;
unsigned char syncSend;


#define MAX_DATA 1018
#define MAX_SENDS 5


void constructPacket( unsigned char* packet, size_t maxSends ); 
void readPacket( unsigned char* packet );
void setCRC( unsigned char* packet );
std::string trimResponse( unsigned char* response );


#endif
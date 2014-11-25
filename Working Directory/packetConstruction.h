#ifndef PACKET_CONSTRUCTION
#define PACKET_CONSTRUCTION

#include <string>
#include <iostream>

#include "crc.h"


std::string buffer;
unsigned char syncSend;


const size_t MAX_DATA = 1018;
#define MAX_SENDS 5


const unsigned char ENQ = 0x05;
const unsigned char ACK = 0x06;
const unsigned char NAK = 0x15;
const unsigned char ETB = 0x17;
const unsigned char ETX = 0x03;
const unsigned char EOT = 0x04;
const unsigned char DC1 = 0x11;
const unsigned char DC2 = 0x12;  // syncSend
const unsigned char DC3 = 0x13;  // syncSend

void printThis( unsigned char* packet, size_t f )
{
	for(size_t i = 0; i < f; i++)
	{
		std::cout << packet[i];
	}
	std::cout << std::endl << std::endl << "+=+=+=+=+=+=+=+=+=+=" << std::endl << std::endl;
}



void constructPacket( unsigned char* packet, size_t maxSends ); 
void readPacket( unsigned char* packet );
void printchar( unsigned char c );
void setCRC( unsigned char* packet );
std::string trimResponse( unsigned char* response );

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: construcPacket
--
-- DATE: November 14, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun, Chris Klassen, Thomas Tallentire, Lewis Scott
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: void constructPacket( unsigned char* packet, size_t maxSends ) 
--			This function constructs a packet for the Grapefruit protocol.  The packet has protocol compliant control
--			characters, has up to 1018 bytes of data, padded with ETX characters if there is less data.  It then has
--			a 32 bit CRC appended to the last four bytes in setCRC();
-- RETURNS: void
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
void constructPacket( unsigned char* packet, size_t maxSends ) 
{
	//should check this at a higher level, so they know
	if( maxSends >= MAX_SENDS )
		return;
		
	if( buffer.size() > MAX_DATA )
	{
		packet[0] = ETB;
		packet[1] = syncSend;
		
		for( size_t i = 0; i < MAX_DATA; i++ )
		{
			packet[i + 2] = (unsigned char)buffer[i];
		}
	}
	else
	{
		packet[0] = EOT;
		packet[1] = syncSend;
		size_t i = 0;
		for(; i < buffer.size(); i++ )
		{
			packet[i + 2] = buffer[i];
		}
		for(; i < MAX_DATA; i++)
		{
			packet[i + 2] = ETX;
		}
	}
	
	syncSend = ( syncSend == DC2 ) ? DC3 : DC2;
	setCRC( packet );
	
	return;
}

void setCRC( unsigned char* packet )
{
	crc c = crcFast( &packet[2], MAX_DATA );
	
	unsigned char* crcArray = new unsigned char[4];
	crcArray = reinterpret_cast<unsigned char *>(&c);	

	for(size_t i = 0; i < 4; i++)
	{
		packet[ MAX_DATA + 2 + i ] = crcArray[i];
	}

	return;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: trimResponse
--
-- DATE: November 18, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun, Chris Klassen, Thomas Tallentire, Lewis Scott
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: string constructPacket( unsigned char* response ) 
--			This function takes a Grapefruit Protocol packet.  This packet is assumed to be pre-validated, meaning
--			that it has passed CRC validation has been confirmed to have the correct EOT or ETB prefix, and has the
--			appropriate sync byte in as its second character.  It returns a string made up of the data in the packet,
--			either to ETX, or the full 1018 bytes of data.
-- RETURNS: string containing all data in the packet.
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/

std::string trimResponse( unsigned char* response )
{
	std::string data;
	data.reserve(MAX_DATA);
	
	data = "";
	for( size_t i = 2; response[i] != ETX && i < MAX_DATA + 2; i++)
	{
		data += response[i];
	}
	data.shrink_to_fit();

	return data;
}

bool validateCRC( unsigned char* response )
{
	crc* c = reinterpret_cast<crc *>( &response[ MAX_DATA + 2 ] );
	
	return *c == crcFast( &response[2], MAX_DATA );
}

void readPacket( unsigned char* packet )
{
	for( size_t i = 0; i < 1019; i++)
	{
		printchar( packet[i] );
	}
	std::cout << std::endl;
}


void printchar( unsigned char c )
{
	using std::cout;
	switch(c)
	{
		case ENQ:
			cout << "ENQ ";
			break;
		case ACK:
			cout << "ACK ";
			break;
		case NAK:
			cout << "NAK ";
			break;
		case ETB:
			cout << " ETB ";
			break;
		case ETX:
			cout << " ETX ";
			break;
		case EOT:
			cout << " EOT ";
			break;
		case DC3:
			cout << "DC3 ";
			break;
		case DC1:
			cout << "DC1 ";
			break;
		case DC2:
			cout << "DC2 ";
			break;
		default:
			cout << c;
			break;
	}
	
	return;
}
#endif
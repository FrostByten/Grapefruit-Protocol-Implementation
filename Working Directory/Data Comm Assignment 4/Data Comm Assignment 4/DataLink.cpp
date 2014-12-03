/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: DataLink.cpp -
--
-- PROGRAM: Irregardless
--
-- FUNCTIONS:
--		
--
-- DATE: November 26, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun & Thomas Tallentire
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/

#include "DataLink.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: constructPacket
--
-- DATE: November 14, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun & Thomas Tallentire
--
-- INTERFACE: void constructPacket( unsigned char* packet, size_t maxSends ) 
--			
-- RETURNS: void
--
-- NOTES:
-- This function constructs a packet for the Grapefruit protocol.  The packet has protocol compliant control
-- characters, has up to 1018 bytes of data, padded with ETX characters if there is less data.  It then has
-- a 32 bit CRC appended to the last four bytes in setCRC();
----------------------------------------------------------------------------------------------------------------------*/
DWORD constructPacket( unsigned char* packet, BOOL maxSent ) 
{
	DWORD totalData = getBufferSize();
	DWORD dataSent = 0;
	//should check this at a higher level, so they know		
	packet[1] = syncSend;
	if( totalData > MAX_DATA )
	{
		if (maxSent)
			packet[0] = EOT;
		else
			packet[0] = ETB;
		
		for( size_t i = 0; i < MAX_DATA; i++ )
		{
			packet[i + 2] = (unsigned char)sendBuffer[i];
		}
		dataSent = MAX_DATA;
	}
	else
	{
		packet[0] = EOT;
		size_t i = 0;
		for(; i < totalData; i++ )
		{
			packet[i + 2] = (unsigned char)sendBuffer[i];
		}
		for(; i < MAX_DATA; i++)
		{
			packet[i + 2] = ETX;
		}
		dataSent = totalData;
	}
	
	syncSend = ( syncSend == SYN1 ) ? SYN2 : SYN1;
	setCRC( packet );
	
	return dataSent;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setCRC
--
-- DATE: November 18, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: void setCRC( unsigned char* packet ) 
--			
--
-- RETURNS: void
--
-- NOTES:
-- This function makes a 32 bit CRC for the message portion ( starting at packet[2] to packet[1020] )
----------------------------------------------------------------------------------------------------------------------*/
void setCRC( unsigned char* packet )
{
	stringstream ss;
	char* test = new char[1024];
	string s;
	
	crc c = crcFast( &packet[2], MAX_DATA );

	/*
	ss << "CRC value " << c;
	s.assign(ss.str());
	int i;
	for (i = 0; i < s.length(); i++)
		{
		test[i] = s[i];
		}
	test[i] = '\0';
	printDebugString(test); */
	
	unsigned char* crcArray = new unsigned char[4];
	crcArray = reinterpret_cast<unsigned char *>(&c);	

	OutputDebugString("Computing CRC: ");
	stringstream sn;
	ss << crcArray;
	OutputDebugString(sn.str().c_str());

	for(size_t i = 0; i < 4; i++)
	{
		packet[ MAX_DATA + 2 + i ] = crcArray[i];
	}

	crc* cc = reinterpret_cast<crc *>(&packet[MAX_DATA + 2]);

	return;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: trimResponse
--
-- DATE: November 18, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: string trimResponse( unsigned char* response )
--			
-- RETURNS: string containing all data in the packet.
--
-- NOTES:
-- This function takes a Grapefruit Protocol packet.  This packet is assumed to be pre-validated, meaning
-- that it has passed CRC validation has been confirmed to have the correct EOT or ETB prefix, and has the
-- appropriate sync byte in as its second character.  It returns a string made up of the data in the packet,
-- either to ETX, or the full 1018 bytes of data.
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: validateCRC
--
-- DATE: November 18, 2014
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun & Thomas Tallentire
--
-- INTERFACE: bool setCRC( unsigned char* response ) 
--			
--
-- RETURNS: true if recieved CRC is valid
--
-- NOTES:
-- calculates the 32 bit crc for the data portion of the response (response[2] to response[1020])
-- and compares it to the crc received with the response.  If they are equal it returs true else false.
----------------------------------------------------------------------------------------------------------------------*/
bool validatePacket( unsigned char* response )
{
	//printDebugString("if this shows... it is doing crc stuff!");

	/* char* yy = new char[2048];
	int f;
	for (f = 0; (f < MAX_DATA + 2) && response[f+2] != ETX; f++)
	{
		yy[f] = (char)response[f+2];
	}
	yy[f] = '\0';
	printDebugString(yy); */

	if (response[1] != syncRx)
		return false;
	crc* c = reinterpret_cast<crc *>( &response[ MAX_DATA + 2 ] );

	stringstream ss;
	ss << "CRC received " << *c << "  CRC calculated " << crcFast(&response[2], MAX_DATA);

	/*string sx = ss.str();
	char* xx = new char[2048];
	int i;
	for (i = 0; i < sx.size(); i++)
	{
		xx[i] = sx[i];
	}
	xx[i] = '\0';*/

	//printDebugString((char*)ss.str().c_str());
	
	return *c == crcFast( &response[2], MAX_DATA );
}
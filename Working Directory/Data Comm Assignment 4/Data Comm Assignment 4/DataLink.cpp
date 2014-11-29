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
-- FUNCTION: construcPacket
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
void constructPacket( unsigned char* packet, size_t maxSends ) 
{
	DWORD totalData = getBufferSize();
	//should check this at a higher level, so they know		
	if( totalData > MAX_DATA )
	{
		packet[0] = ETB;
		packet[1] = syncSend;
		
		for( size_t i = 0; i < MAX_DATA; i++ )
		{
			packet[i + 2] = (unsigned char)sendBuffer[i];
		}
	}
	else
	{
		packet[0] = EOT;
		packet[1] = syncSend;
		size_t i = 0;
		for(; i < totalData; i++ )
		{
			packet[i + 2] = sendBuffer[i];
		}
		for(; i < MAX_DATA; i++)
		{
			packet[i + 2] = ETX;
		}
	}
	
	syncSend = ( syncSend == SYN1 ) ? SYN2 : SYN1;
	setCRC( packet );
	
	return;
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
-- This function makes a 32 bit CRC for the message portion ( starting at packet[2] to packet[1020]
----------------------------------------------------------------------------------------------------------------------*/
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
	if (response[1] != syncRx)
		return false;
	crc* c = reinterpret_cast<crc *>( &response[ MAX_DATA + 2 ] );
	
	return *c == crcFast( &response[2], MAX_DATA );
}
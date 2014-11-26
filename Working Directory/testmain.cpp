#include "packetConstruction.h"
#include "crc.h"
#include <sstream>
#include <string>
#include <bitset>


using namespace std;

int main()
{
	unsigned char packet[1019];
	string stars( 80, '*' );
	
	crcInit();
	syncSend = DC2;
	
	buffer.clear();
	buffer = "123456789";
	
	unsigned char test[] = "123456789";
	unsigned char* x = new unsigned char[4];
	unsigned char result[13];
	for(int i = 0; i < 13; i++)
		result[i] = 'x';
	
	crc c = crcFast(test, 9);
	x = reinterpret_cast<unsigned char *>(&c);	
	result[9] = x[0];
	result[10] = x[1];
	result[11] = x[2];
	result[12] = x[3];
	for(int i = 0; i < 13; i++)
		cout << ( (result[i] == 'x') ? 'x':'-' );
	cout << endl;
	
	for(size_t i = 0; test[i] != '\0'; i++)
	{
		result[i] = test[i];
	}
	
	
	crc cResult = crcFast(result, 13);
	crc * myHope = reinterpret_cast<crc *>( &result[9] );
	

	cout << "Value crced " << test << endl;
	cout << "Expected result " << CHECK_VALUE << endl;
	cout << "CRCFast result  " << c << endl;
	cout << "CRC from whole shebang " << cResult << endl;
	cout << "CRC from myHope " << *myHope << endl << endl << endl;

	
	constructPacket(packet, 2);
	if( validateCRC( packet ) )
		cout << trimResponse(packet) << endl;
	else
		cout << "FFFFUUUUUUUUUUUUUUUUUUUUUUUUCK" << endl;
		
	cout << endl << endl;
	
//	cout << endl << stars << endl << endl;
	
	buffer.clear();
	buffer = "First, buying real estate without an adequate down payment means extreme leverage, which brings added risk. No, houses do not go up forever. Therefore, buying with 5% down and twenty-times leverage means a lowly 10% correction in the market would wipe out all your savings. You’d owe more than the property’s worth. If you don’t think that’s possible, come back this time next year. We’ll talk.\n\nThen there’s the CMHC premium, which is north of 3% of the mortgaged amount for a minimal down payment. On a $500,000 mortgage, the cost is almost $16,000. Most people add this to their mortgage, so it gets amortized and effectively doubled over time. Your property then needs to appreciate an extra $30,000 just to break even. Bummer.\n\nIn short, a big down is good. This woman gets it. But what sense does it make to keep $200,000 in the tangerine guy’s shorts at 1.3%, when the inflation rate is 2.03% and all of the interest is taxable at your marginal rate? And what if house prices in your hood rise by 5% while you’re saving?\nRight. Fail. That money is actually shrinking due to inflation and taxes while you sit on it. Even at double or triple the interest rate, you’re falling further behind. And if you lock the cash into a five-year GIC to boost the return to 2.8% at some godforsaken online Lithuanian steelworkers’ benevolent Manitoba credit union, it’s not available to you should a great house deal materialize in 20 months from now.";
	
	constructPacket(packet, 2);
	if( validateCRC( packet ) )
		cout << trimResponse(packet) << endl;
	else
		cout << "FFFFUUUUUUUUUUUUUUUUUUUUUUUUCK" << endl;
		
	cout << endl << endl;
	
	cout << endl << stars << endl << endl;
	buffer.erase(0, MAX_DATA);
	
	constructPacket(packet, 2);
	if( validateCRC( packet ) )
		cout << trimResponse(packet) << endl;
	else
		cout << "FFFFUUUUUUUUUUUUUUUUUUUUUUUUCK" << endl;
		
	cout << endl << endl;
	
	return 0;
	
	
}
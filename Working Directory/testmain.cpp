#include "packetConstruction.h"
using namespace std;
int main()
{
	unsigned char packet[1019];
	string stars( 80, '*' );
	
	syncSend = DC2;
	buffer.clear();
	buffer = "hello";
	
	constructPacket(packet, 2);
	readPacket(packet);
	
	cout << endl << stars << endl << endl;
	
	buffer.clear();
	buffer = "First, buying real estate without an adequate down payment means extreme leverage, which brings added risk. No, houses do not go up forever. Therefore, buying with 5% down and twenty-times leverage means a lowly 10% correction in the market would wipe out all your savings. You’d owe more than the property’s worth. If you don’t think that’s possible, come back this time next year. We’ll talk.\n\nThen there’s the CMHC premium, which is north of 3% of the mortgaged amount for a minimal down payment. On a $500,000 mortgage, the cost is almost $16,000. Most people add this to their mortgage, so it gets amortized and effectively doubled over time. Your property then needs to appreciate an extra $30,000 just to break even. Bummer.\n\nIn short, a big down is good. This woman gets it. But what sense does it make to keep $200,000 in the tangerine guy’s shorts at 1.3%, when the inflation rate is 2.03% and all of the interest is taxable at your marginal rate? And what if house prices in your hood rise by 5% while you’re saving?\nRight. Fail. That money is actually shrinking due to inflation and taxes while you sit on it. Even at double or triple the interest rate, you’re falling further behind. And if you lock the cash into a five-year GIC to boost the return to 2.8% at some godforsaken online Lithuanian steelworkers’ benevolent Manitoba credit union, it’s not available to you should a great house deal materialize in 20 months from now.";
	
	constructPacket(packet, 2);
	readPacket(packet);
	
	cout << endl << stars << endl << endl;
	buffer.erase(0, MAX_DATA);
	
	constructPacket(packet, 2);
	readPacket(packet);
	
	return 0;
	
	
}
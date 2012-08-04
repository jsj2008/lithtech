#include "WONAPI.h"
#include "WONServer/PingOp.h"
#include <iostream>

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONAPICore gAPI(false);


const int DEFAULT_RETURN = -1;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Usage()
{
	cout << "Usage: ServerPing [-l size] [-p pause] [-r | -t] [-w timeout] server-address" << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << "    -l size      Length field size in bytes (Def: 4)" << endl;
	cout << "    -n count     Number of pings (Def: 4)" << endl;
	cout << "    -p pause     Time in milliseconds to wait between pings (Def: 1000)" << endl;
	cout << "    -r           Return the latency of the last ping" << endl;
	cout << "    -t           Ping the specified server until interrupted" << endl;
	cout << "    -w timeout   Timeout in milliseconds to wait for each reply (Def: 3000)" << endl;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	unsigned long aNumPings = 4;
	bool          fReturn = false;
	unsigned long aLengthFieldSize = 4;
	long          aTimeout = 3000;
	unsigned long aPause = 1000;
	unsigned long anAddrArg = 0;
	int i=0;

	// parse arguments
	for (i = 1; i < argc; ++i)
	{
		if (strlen(argv[i]) >= 2 && (argv[i][0] == '-' || argv[i][0] == '/'))
		{
			char aSwitch = toupper(argv[i][1]);
			switch (aSwitch)
			{
				// options that stand on their own
				case 'R': fReturn = true;        break;
				case 'T': aNumPings = ULONG_MAX; break; // FIX: not really "forever"
				case '?': Usage();               return DEFAULT_RETURN;

				// options that take parameters
				default:
				{
					if (argc == i + 1)
					{
						cout << "Missing parameter on last switch." << endl << endl;
						Usage();
						return DEFAULT_RETURN;
					}

					char* aParam = argv[++i];
					switch (aSwitch)
					{
						case 'L': aLengthFieldSize = atoi(aParam); break;
						case 'N': aNumPings = atoi(aParam);        break;
						case 'P': aPause = atoi(aParam);           break;
						case 'W': aTimeout = atoi(aParam);         break;
					}
				}
			}
		}
		else
			anAddrArg = i;
	}

	// get address of server to ping
	IPAddr anAddr;
	anAddr.SetRememberHostString(true);
	if (anAddrArg == 0)
	{
		cout << "Must specify an address." << endl << endl;
		Usage();
		return DEFAULT_RETURN;
	}
	else
		anAddr.Set(argv[anAddrArg]);

	if(!anAddr.IsValid())
	{
		cout << "Invalid address: " << anAddr.GetHostAndPortString(true) << endl;
		return DEFAULT_RETURN;
	}

	// output header
	cout << "Pinging " << anAddr.GetHostAndPortString(true) << " [" << anAddr.GetHostAndPortString(false) << "]:" << endl << endl;

	// do the pinging
	WONStatus aStatus = WS_None;
	unsigned int aPingTime;
	for (i = 0; i < aNumPings; ++i)
	{
		// do a single ping
		PingOpPtr anOp = new PingOp(anAddr, aLengthFieldSize, false);
		aStatus = anOp->Run(OP_MODE_BLOCK, aTimeout);
		aPingTime = anOp->mLag;
		
		// output results of this ping
		switch (aStatus)
		{
			case WS_Success:
				cout << "Reply from " << anAddr.GetHostAndPortString(false) << ": " << aPingTime << "ms"; break;
			case WS_TimedOut:
				cout << "Request timed out"; break;
			default:
				cout << "Error: " << WONStatusToString(aStatus); break;
		}
		cout << endl;

		// pause before next ping
		if (aPause) Sleep(aPause); // do the if so we don't even give up our time slice if aPause == 0.
	}

	if (!fReturn || aStatus != WS_Success)
		return DEFAULT_RETURN;
	else
		return aPingTime;
}


#ifndef __WON_DetectFirewallOP_H__
#define __WON_DetectFirewallOP_H__
#include "WONShared.h"
#include "WONServer/ServerRequestOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class DetectFirewallOp : public ServerRequestOp
{
private:
	unsigned short mListenPort;
	unsigned long mMaxConnectWaitTime;
	BlockingSocketPtr mListenSocket;
	bool	mDoListen;
	bool	mUseUDP;
	RecvBytesFromOpPtr	mRecvBytesFromOp;

	void Init();

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();
	virtual void CleanupHook();
	virtual void RunHook();
	virtual bool CallbackHook(AsyncOp *theOp, int theParam);
	virtual WONStatus SetupReceiveTracker();

	enum DetectFirewallTrack
	{
		DetectFirewall_Track_RecvFrom = 1000
	};

public:
	DetectFirewallOp(ServerContext *theFirewallDetectorContext);
	DetectFirewallOp(const IPAddr &theAddr);

	void SetDoUDPDetect(bool useUDP)	{ mUseUDP = useUDP; }
	void SetListenPort(unsigned short thePort) { mListenPort = thePort;  }
	void SetBindPort  (unsigned short thePort) { SetListenPort(thePort); }
	void SetMaxConnectWaitTime(unsigned long theNumSeconds) { mMaxConnectWaitTime = theNumSeconds; }
	void SetDoListen(bool doListen) { mDoListen = doListen; }
};

typedef SmartPtr<DetectFirewallOp> DetectFirewallOpPtr;

}; // namespace WONAPI

#endif

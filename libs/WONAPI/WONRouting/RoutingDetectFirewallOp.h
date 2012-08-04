#ifndef __WON_ROUTINGDETECTFIREWALLOP_H__
#define __WON_ROUTINGDETECTFIREWALLOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingDetectFirewallOp : public RoutingOp
{
private:
	unsigned short mListenPort;
	unsigned long mMaxConnectWaitTime;
	BlockingSocketPtr mListenSocket;
	bool mDoListen;
	bool mWaitingForStatusReply;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);
	virtual void CleanupHook();

public:
	RoutingDetectFirewallOp(RoutingConnection *theConnection);

	void SetListenPort(unsigned short thePort) { mListenPort = thePort; }
	void SetMaxConnectWaitTime(unsigned long theNumSeconds) { mMaxConnectWaitTime = theNumSeconds; }
	void SetDoListen(bool doListen) { mDoListen = doListen; }

	unsigned short GetListenPort() const { return mListenPort; }
	unsigned long GetMaxConnectWaitTime() const { return mMaxConnectWaitTime; }
	bool GetDoListen() { return mDoListen; }

	virtual RoutingOpType GetType() const { return RoutingOp_DetectFirewall; }
};


typedef SmartPtr<RoutingDetectFirewallOp> RoutingDetectFirewallOpPtr;


}; // namespace WONAPI



#endif

#ifndef __WON_ROUTINGABORTSERVERSHUTDOWNOP_H__
#define __WON_ROUTINGABORTSERVERSHUTDOWNOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingAbortServerShutdownOp : public RoutingOp
{
private:
	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingAbortServerShutdownOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	virtual RoutingOpType GetType() const { return RoutingOp_AbortServerShutdown; }
};


typedef SmartPtr<RoutingAbortServerShutdownOp> RoutingAbortServerShutdownOpPtr;


}; // namespace WONAPI



#endif

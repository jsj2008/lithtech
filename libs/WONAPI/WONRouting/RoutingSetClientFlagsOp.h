#ifndef __WON_ROUTINGSETCLIENTFLAGSOP_H__
#define __WON_ROUTINGSETCLIENTFLAGSOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingSetClientFlagsOp : public RoutingOp
{
private:
	unsigned long mClientFlagMask;
	unsigned long mClientFlags;

	unsigned long mAsyncMessageFlagMask;
	unsigned long mAsyncMessageFlags;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingSetClientFlagsOp(RoutingConnection *theConnection, unsigned short theClientFlagMask = 0, unsigned long theClientFlags = 0, unsigned long theAsyncMessageFlagMask = 0, unsigned long theAsyncMessageFlags = 0) : RoutingOp(theConnection), mClientFlagMask(theClientFlagMask), mClientFlags(theClientFlags), mAsyncMessageFlagMask(theAsyncMessageFlagMask), mAsyncMessageFlags(theAsyncMessageFlags) {}

	void SetClientFlagMask(unsigned long theFlagMask) { mClientFlagMask = theFlagMask; }
	void SetClientFlags(unsigned long theFlags) { mClientFlags = theFlags; }
	void SetAsyncMessageFlagMask(unsigned long theFlagMask) { mAsyncMessageFlagMask = theFlagMask; }
	void SetAsyncMessageFlags(unsigned long theFlags) { mAsyncMessageFlags = theFlags; }

	unsigned long GetClientFlagMask() { return mClientFlagMask; }
	unsigned long GetClientFlags() { return mClientFlags; }
	unsigned long GetAsyncMessageFlagMask() { return mAsyncMessageFlagMask; }
	unsigned long GetAsyncMessageFlags() { return mAsyncMessageFlags; }

	virtual RoutingOpType GetType() const { return RoutingOp_SetClientFlags; }
};


typedef SmartPtr<RoutingSetClientFlagsOp> RoutingSetClientFlagsOpPtr;


}; // namespace WONAPI



#endif

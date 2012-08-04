#ifndef __WON_ROUTINGCLIENTFLAGSCHANGEDOP_H__
#define __WON_ROUTINGCLIENTFLAGSCHANGEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingClientFlagsChangedOp : public RoutingOp
{
private:
	unsigned short mClientId;
	unsigned long mNewClientFlags;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingClientFlagsChangedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetClientId() const { return mClientId; }
	unsigned long GetNewClientFlags() const { return mNewClientFlags; }
	
	bool GetIsModerator() const { return (mNewClientFlags & RoutingClientFlag_IsModerator) != 0; }
	bool GetIsMuted() const     { return (mNewClientFlags & RoutingClientFlag_IsMuted) != 0; }
	bool GetIsAway() const      { return (mNewClientFlags & RoutingClientFlag_IsAway) != 0; }

	virtual RoutingOpType GetType() const { return RoutingOp_ClientFlagsChanged; }
};

typedef SmartPtr<RoutingClientFlagsChangedOp> RoutingClientFlagsChangedOpPtr;

}; // namespace WONAPI


#endif

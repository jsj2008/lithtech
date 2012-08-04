#ifndef __WON_ROUTINGCLIENTJOINEDSERVEROP_H__
#define __WON_ROUTINGCLIENTJOINEDSERVEROP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingClientJoinedServerOp : public RoutingOp
{
private:
	unsigned short mClientId;
	std::wstring mClientName;
	unsigned long mClientFlags;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingClientJoinedServerOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetClientId() const { return mClientId; }
	const std::wstring& GetClientName() const { return mClientName; }
	unsigned long GetClientFlags() const { return mClientFlags; }

	bool GetIsModerator() const { return (mClientFlags & RoutingClientFlag_IsModerator) != 0; }
	bool GetIsMuted() const     { return (mClientFlags & RoutingClientFlag_IsMuted) != 0; }
	bool GetIsAway() const      { return (mClientFlags & RoutingClientFlag_IsAway) != 0; }

	virtual RoutingOpType GetType() const { return RoutingOp_ClientJoinedServer; }
};

typedef SmartPtr<RoutingClientJoinedServerOp> RoutingClientJoinedServerOpPtr;

}; // namespace WONAPI


#endif

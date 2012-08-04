#ifndef __WON_ROUTINGCLIENTLEFTGROUPOP_H__
#define __WON_ROUTINGCLIENTLEFTGROUPOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingClientLeftGroupOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mClientId;
	unsigned char mMemberFlags;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingClientLeftGroupOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetClientId() const { return mClientId; }

	virtual RoutingOpType GetType() const { return RoutingOp_ClientLeftGroup; }
};

typedef SmartPtr<RoutingClientLeftGroupOp> RoutingClientLeftGroupOpPtr;

}; // namespace WONAPI


#endif

#ifndef __WON_ROUTINGGROUPMEMBERCOUNTOP_H__
#define __WON_ROUTINGGROUPMEMBERCOUNTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGroupMemberCountOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mMemberCount;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGroupMemberCountOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetMemberCount() const { return mMemberCount; }

	virtual RoutingOpType GetType() const { return RoutingOp_GroupMemberCount; }
};

typedef SmartPtr<RoutingGroupMemberCountOp> RoutingGroupMemberCountOpPtr;

}; // namespace WONAPI


#endif

#ifndef __WON_ROUTINGMEMBERFLAGSCHANGEDOP_H__
#define __WON_ROUTINGMEMBERFLAGSCHANGEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingMemberFlagsChangedOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mClientId;
	unsigned char mNewMemberFlags;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingMemberFlagsChangedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetClientId() const { return mClientId; }
	unsigned char GetNewMemberFlags() const { return mNewMemberFlags; }

	bool GetIsModerator() const { return (mNewMemberFlags & RoutingMemberFlag_IsModerator) != 0; }
	bool GetIsMuted() const     { return (mNewMemberFlags & RoutingMemberFlag_IsMuted) != 0; }
	bool GetIsObserver() const  { return (mNewMemberFlags & RoutingMemberFlag_IsObserver) != 0; }

	virtual RoutingOpType GetType() const { return RoutingOp_MemberFlagsChanged; }
};

typedef SmartPtr<RoutingMemberFlagsChangedOp> RoutingMemberFlagsChangedOpPtr;

}; // namespace WONAPI


#endif

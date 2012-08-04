#ifndef __WON_ROUTINGCLIENTJOINEDGROUPOP_H__
#define __WON_ROUTINGCLIENTJOINEDGROUPOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingClientJoinedGroupOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mClientId;
	unsigned char mMemberFlags;
	std::wstring mClientName;
	unsigned long mClientFlags;
	bool mHasClientFlags;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingClientJoinedGroupOp(RoutingConnection *theConnection) : RoutingOp(theConnection), mClientFlags(0), mHasClientFlags(false) {}

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetClientId() const { return mClientId; }
	unsigned char GetMemberFlags() const { return mMemberFlags; }

	const std::wstring& GetClientName() const { return mClientName; } // may not come with message (empty -> didn't come)
	unsigned long GetClientFlags() const { return mClientFlags; } // may not come with message
	bool HasClientFlags() const { return mHasClientFlags; }	

	bool GetIsModerator() const { return (mMemberFlags & RoutingMemberFlag_IsModerator) != 0; }
	bool GetIsMuted() const     { return (mMemberFlags & RoutingMemberFlag_IsMuted) != 0; }
	bool GetIsObserver() const  { return (mMemberFlags & RoutingMemberFlag_IsObserver) != 0; }

	virtual RoutingOpType GetType() const { return RoutingOp_ClientJoinedGroup; }
};

typedef SmartPtr<RoutingClientJoinedGroupOp> RoutingClientJoinedGroupOpPtr;

}; // namespace WONAPI


#endif

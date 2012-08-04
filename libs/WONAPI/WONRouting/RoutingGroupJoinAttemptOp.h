#ifndef __WON_ROUTINGGROUPJOINATTEMPTOP_H__
#define __WON_ROUTINGGROUPJOINATTEMPTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGroupJoinAttemptOp : public RoutingOp
{
private:
	unsigned short mClientId;
	unsigned short mGroupId;
	std::wstring mComment;
	unsigned char mJoinGroupFlags;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGroupJoinAttemptOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetClientId() const { return mClientId; }
	unsigned short GetGroupId() const { return mGroupId; }
	const std::wstring& GetComment() const { return mComment; }
	unsigned char GetJoinGroupFlags() const { return mJoinGroupFlags; }

	virtual RoutingOpType GetType() const { return RoutingOp_GroupJoinAttempt; }
};

typedef SmartPtr<RoutingGroupJoinAttemptOp> RoutingGroupJoinAttemptOpPtr;

}; // namespace WONAPI


#endif

#ifndef __WON_ROUTINGGROUPINVITATIONOP_H__
#define __WON_ROUTINGGROUPINVITATIONOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGroupInvitationOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mCaptainId;
	bool mAmInvited;
	std::wstring mComment;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGroupInvitationOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetCaptainId() const { return mCaptainId; }
	bool GetAmInvited() const { return mAmInvited; }
	const std::wstring& GetComment() const { return mComment; }

	virtual RoutingOpType GetType() const { return RoutingOp_GroupInvitation; }
};

typedef SmartPtr<RoutingGroupInvitationOp> RoutingGroupInvitationOpPtr;

}; // namespace WONAPI


#endif

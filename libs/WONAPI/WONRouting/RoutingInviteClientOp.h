#ifndef __WON_ROUTINGINVITECLIENTOP_H__
#define __WON_ROUTINGINVITECLIENTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingInviteClientOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mClientId;
	bool mInvited;
	std::wstring mInviteComment;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingInviteClientOp(RoutingConnection *theConnection);

	void SetGroupId(unsigned short theId) { mGroupId = theId; }
	void SetClientId(unsigned short theId) { mClientId = theId; }
	void SetInviteComment(const std::wstring &theComment) { mInviteComment = theComment; }
	void SetInvited(bool isInvited) { mInvited = isInvited; }
	
	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetClientId() const { return mClientId; }
	bool GetInvited() const { return mInvited; }
	const std::wstring& GetInviteComment() const { return mInviteComment; }

	virtual RoutingOpType GetType() const { return RoutingOp_InviteClient; }
};


typedef SmartPtr<RoutingInviteClientOp> RoutingInviteClientOpPtr;


}; // namespace WONAPI



#endif

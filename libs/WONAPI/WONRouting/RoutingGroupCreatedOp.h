#ifndef __WON_ROUTINGGROUPCREATEDOP_H__
#define __WON_ROUTINGGROUPCREATEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGroupCreatedOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	std::wstring mGroupName;
	unsigned short mMaxPlayers;
	unsigned long mGroupFlags;
	unsigned long mAsyncFlags;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGroupCreatedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	const std::wstring& GetGroupName() const { return mGroupName; }
	unsigned short GetMaxPlayers() const { return mMaxPlayers; }
	unsigned long GetGroupFlags() const { return mGroupFlags; }
	unsigned long GetAsyncFlags() const { return mAsyncFlags; }

	bool GetIsClaimed() const            { return (mGroupFlags & RoutingGroupFlag_Claimed) != 0; }
	bool GetIsClosed() const             { return (mGroupFlags & RoutingGroupFlag_Closed) != 0; }
	bool GetIsPasswordProtected() const  { return (mGroupFlags & RoutingGroupFlag_PasswordProtected) != 0; }
	bool GetAllowOutsidersToChat() const { return (mGroupFlags & RoutingGroupFlag_AllowOutsidersToChat) != 0; }
	bool GetAllowObservers() const       { return (mGroupFlags & RoutingGroupFlag_AllowObservers) != 0; }
	bool GetAllowObserversToChat() const { return (mGroupFlags & RoutingGroupFlag_AllowObserversToChat) != 0; }
	bool GetDetailedObservers() const    { return (mGroupFlags & RoutingGroupFlag_DetailedObservers) != 0; }
	bool GetIsInviteOnly() const         { return (mGroupFlags & RoutingGroupFlag_InviteOnly) != 0; }
	bool GetAskCaptainToJoin() const     { return (mGroupFlags & RoutingGroupFlag_AskCaptainToJoin) != 0; }
	bool GetAskCaptainToObserve() const  { return (mGroupFlags & RoutingGroupFlag_AskCaptainToObserve) != 0; }
	bool GetNoCaptain() const            { return (mGroupFlags & RoutingGroupFlag_NoCaptain) != 0; }
	bool GetIsChatRoom() const           { return (mGroupFlags & RoutingGroupFlag_IsChatRoom) != 0; }

	virtual RoutingOpType GetType() const { return RoutingOp_GroupCreated; }
};

typedef SmartPtr<RoutingGroupCreatedOp> RoutingGroupCreatedOpPtr;

}; // namespace WONAPI


#endif

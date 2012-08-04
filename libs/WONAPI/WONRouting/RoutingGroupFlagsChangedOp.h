#ifndef __WON_ROUTINGGROUPFLAGSCHANGEDOP_H__
#define __WON_ROUTINGGROUPFLAGSCHANGEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGroupFlagsChangedOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned long mNewGroupFlags;
	unsigned long mNewAsyncFlags;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGroupFlagsChangedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned long GetNewGroupFlags() const { return mNewGroupFlags; }
	unsigned long GetNewAsyncFlags() const { return mNewAsyncFlags; }

	bool GetIsClaimed() const            { return (mNewGroupFlags & RoutingGroupFlag_Claimed) != 0; }
	bool GetIsClosed() const             { return (mNewGroupFlags & RoutingGroupFlag_Closed) != 0; }
	bool GetIsPasswordProtected() const  { return (mNewGroupFlags & RoutingGroupFlag_PasswordProtected) != 0; }
	bool GetAllowOutsidersToChat() const { return (mNewGroupFlags & RoutingGroupFlag_AllowOutsidersToChat) != 0; }
	bool GetAllowObservers() const       { return (mNewGroupFlags & RoutingGroupFlag_AllowObservers) != 0; }
	bool GetAllowObserversToChat() const { return (mNewGroupFlags & RoutingGroupFlag_AllowObserversToChat) != 0; }
	bool GetDetailedObservers() const    { return (mNewGroupFlags & RoutingGroupFlag_DetailedObservers) != 0; }
	bool GetIsInviteOnly() const         { return (mNewGroupFlags & RoutingGroupFlag_InviteOnly) != 0; }
	bool GetAskCaptainToJoin() const     { return (mNewGroupFlags & RoutingGroupFlag_AskCaptainToJoin) != 0; }
	bool GetAskCaptainToObserve() const  { return (mNewGroupFlags & RoutingGroupFlag_AskCaptainToObserve) != 0; }
	bool GetNoCaptain() const            { return (mNewGroupFlags & RoutingGroupFlag_NoCaptain) != 0; }
	bool GetIsChatRoom() const           { return (mNewGroupFlags & RoutingGroupFlag_IsChatRoom) != 0; }
  
	virtual RoutingOpType GetType() const { return RoutingOp_GroupFlagsChanged; }
};

typedef SmartPtr<RoutingGroupFlagsChangedOp> RoutingGroupFlagsChangedOpPtr;

}; // namespace WONAPI


#endif

#include "RoutingJoinGroupOp.h"
#include "RoutingGetMembersOfGroupOp.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingJoinGroupOp::RoutingJoinGroupOp(RoutingConnection *theConnection) : RoutingOp(theConnection)
{
	mGroupId = 0;
	mJoinFlags = 0;
	mMemberFlags = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingJoinGroupOp::SendRequest()
{
	InitSendMsg(RoutingJoinGroupRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendWString(mGroupPassword);
	mSendMsg.AppendWString(mJoinComment);
	mSendMsg.AppendByte(mJoinFlags);
	mSendMsg.AppendByte(mMemberFlags);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingJoinGroupOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType==RoutingJoinGroupReply)
	{
		WONStatus aStatus = (WONStatus)theMsg.ReadShort();
		unsigned short aGroupId = theMsg.ReadShort();
		if(aGroupId!=mGroupId) // not a reply for this group
			return WS_RoutingOp_DontWantReply;

		theMsg.ReadWString(mJoinComment);

		if(aStatus==WS_RoutingServG2_Pending)
			return WS_RoutingOp_NeedMoreReplies;
		if(aStatus!=WS_Success)
			return aStatus;
		else if(mJoinFlags & RoutingJoinGroupFlag_GetMembersOfGroup)
			return WS_RoutingOp_NeedMoreReplies;
		else
			return WS_Success;
	}
	else if(theMsgType==RoutingGetMembersOfGroupReply)
	{
		RoutingGetMembersOfGroupOpPtr anOp = new RoutingGetMembersOfGroupOp(mConnection);
		anOp->SetGroupId(mGroupId);
		WONStatus aStatus = anOp->HandleReply(theMsgType,theMsg);
		if(aStatus==WS_Success)
			mMemberMap = anOp->GetMemberMap();

		return aStatus;
	}
	else
		return WS_RoutingOp_DontWantReply;
}


#include "RoutingCancelJoinGroupOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingCancelJoinGroupOp::SendRequest()
{
	InitSendMsg(RoutingCancelJoinGroupRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendBool(mLeaveIfAlreadyInGroup);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingCancelJoinGroupOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


#include "RoutingInviteClientOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingInviteClientOp::RoutingInviteClientOp(RoutingConnection *theConnection) : 
	RoutingOp(theConnection),
	mGroupId(0),
	mClientId(0),
	mInvited(false)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingInviteClientOp::SendRequest()
{
	InitSendMsg(RoutingInviteClientRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendShort(mClientId);
	mSendMsg.AppendBool(mInvited);
	mSendMsg.AppendWString(mInviteComment);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingInviteClientOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


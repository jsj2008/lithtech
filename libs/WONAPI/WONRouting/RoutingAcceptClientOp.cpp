#include "RoutingAcceptClientOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingAcceptClientOp::RoutingAcceptClientOp(RoutingConnection *theConnection) : 
	RoutingOp(theConnection),
	mGroupId(0),
	mClientId(0),
	mAccepted(false)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingAcceptClientOp::SendRequest()
{
	InitSendMsg(RoutingAcceptClientRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendShort(mClientId);
	mSendMsg.AppendBool(mAccepted);
	mSendMsg.AppendWString(mAcceptComment);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingAcceptClientOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


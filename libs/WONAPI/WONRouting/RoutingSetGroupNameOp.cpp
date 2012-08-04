#include "RoutingSetGroupNameOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingSetGroupNameOp::SendRequest()
{
	InitSendMsg(RoutingSetGroupNameRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendWString(mGroupName);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingSetGroupNameOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


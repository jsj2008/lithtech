#include "RoutingSendComplaintOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingSendComplaintOp::SendRequest()
{
/*	InitSendMsg(RoutingSendComplaintRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendShort(mClientId);
	mSendMsg.AppendWString(mText);
	SendMsg();
	AddOp();*/
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingSendComplaintOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


#include "RoutingStartServShutdownOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingStartServerShutdownOp::SendRequest()
{
	InitSendMsg(RoutingStartServerShutdownRequest);
	mSendMsg.AppendWString(mAlertText);
	mSendMsg.AppendLong(mSecondsUntilShutdown);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingStartServerShutdownOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


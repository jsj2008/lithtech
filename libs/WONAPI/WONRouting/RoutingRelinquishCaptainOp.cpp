#include "RoutingRelinquishCaptainOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingRelinquishCaptaincyOp::SendRequest()
{
	InitSendMsg(RoutingRelinquishCaptaincyRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendShort(mNewCaptainId);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingRelinquishCaptaincyOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


#include "RoutingSetClientFlagsOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingSetClientFlagsOp::SendRequest()
{
	InitSendMsg(RoutingSetClientFlagsRequest);
	mSendMsg.AppendLong(mAsyncMessageFlagMask);
	mSendMsg.AppendLong(mAsyncMessageFlags);
	mSendMsg.AppendLong(mClientFlagMask);
	mSendMsg.AppendLong(mClientFlags);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingSetClientFlagsOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


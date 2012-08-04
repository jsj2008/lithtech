#include "RoutingSetGroupFlagsOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingSetGroupFlagsOp::SendRequest()
{
	InitSendMsg(RoutingSetGroupFlagsRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendLong(mGroupFlagMask);
	mSendMsg.AppendLong(mGroupFlags);
	mSendMsg.AppendLong(mAsyncFlagMask);
	mSendMsg.AppendLong(mAsyncFlags);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingSetGroupFlagsOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


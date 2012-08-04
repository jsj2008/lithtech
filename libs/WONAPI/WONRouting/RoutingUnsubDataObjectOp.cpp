#include "RoutingUnsubDataObjectOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingUnsubscribeDataObjectOp::RoutingUnsubscribeDataObjectOp(RoutingConnection *theConnection) : RoutingOp(theConnection)
{
	mLinkId = 0;
	mFlags = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingUnsubscribeDataObjectOp::SendRequest()
{
	InitSendMsg(RoutingUnsubscribeDataObjectRequest);
	mSendMsg.AppendShort(mLinkId);
	mSendMsg.AppendString(mDataType,1);
	mSendMsg.AppendByte(mFlags);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingUnsubscribeDataObjectOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}

#include "RoutingCreateDataObjectOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingCreateDataObjectOp::RoutingCreateDataObjectOp(RoutingConnection *theConnection) : RoutingOp(theConnection)
{
	mLinkId = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingCreateDataObjectOp::SendRequest()
{
	InitSendMsg(RoutingCreateDataObjectRequest);
	mSendMsg.AppendByte(mFlags);
	mSendMsg.AppendShort(mLinkId);
	mSendMsg.AppendString(mDataType,1);
	mSendMsg.AppendWString(mDataName,1);
	mSendMsg.AppendBuffer(mData,2);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingCreateDataObjectOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}

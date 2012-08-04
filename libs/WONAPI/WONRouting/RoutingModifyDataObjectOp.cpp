#include "RoutingModifyDataObjectOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingModifyDataObjectOp::RoutingModifyDataObjectOp(RoutingConnection *theConnection) : RoutingOp(theConnection)
{
	mLinkId = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingModifyDataObjectOp::SendRequest()
{
	InitSendMsg(RoutingModifyDataObjectRequest);
	mSendMsg.AppendShort(mLinkId);
	mSendMsg.AppendString(mDataType,1);
	mSendMsg.AppendWString(mDataName,1);
	mSendMsg.AppendShort(mOffset);
	mSendMsg.AppendBool(mIsInsert);
	mSendMsg.AppendBuffer(mData,2);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingModifyDataObjectOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}

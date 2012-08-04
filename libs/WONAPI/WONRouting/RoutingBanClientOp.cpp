#include "RoutingBanClientOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingBanClientOp::RoutingBanClientOp(RoutingConnection *theConnection) : 
	RoutingOp(theConnection),
	mGroupId(RoutingId_Global),
	mIsBanned(true),
	mBanTime(0),
	mClientId(0),
	mWONId(0),
	mBanByWONId(true)
{
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingBanClientOp::SendRequest()
{
	InitSendMsg(RoutingBanClientRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendBool(mIsBanned);
	mSendMsg.AppendLong(mBanTime);
	mSendMsg.AppendWString(mBanComment);
	if(mBanByWONId)
	{
		mSendMsg.AppendByte(2);	// Content type = WON Id
		mSendMsg.AppendLong(mWONId);
	}
	else
	{
		mSendMsg.AppendByte(1); // Content type = Client Id
		mSendMsg.AppendShort(mClientId);
	}

	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingBanClientOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


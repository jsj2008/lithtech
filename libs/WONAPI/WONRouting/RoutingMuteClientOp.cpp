#include "RoutingMuteClientOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingMuteClientOp::RoutingMuteClientOp(RoutingConnection *theConnection) : 
	RoutingOp(theConnection),
	mGroupId(RoutingId_Global),
	mIsMuted(true),
	mMuteTime(0),
	mClientId(0),
	mWONId(0),
	mMuteByWONId(true)
{
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingMuteClientOp::SendRequest()
{
	InitSendMsg(RoutingMuteClientRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendBool(mIsMuted);
	mSendMsg.AppendLong(mMuteTime);
	mSendMsg.AppendWString(mMuteComment);
	if(mMuteByWONId)
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
WONStatus RoutingMuteClientOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


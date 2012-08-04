#include "RoutingSendDataOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingSendDataOp::SendRequest()
{
	InitSendMsg(RoutingSendDataRequest);
	mSendMsg.AppendByte(mFlags);
	mSendMsg.AppendBuffer(mData,2);
	mSendMsg.AppendShort(mRecipients.size());
	RoutingRecipientList::iterator anItr = mRecipients.begin();
	while(anItr!=mRecipients.end())
	{
		mSendMsg.AppendShort(*anItr);
		++anItr;
	}

	SendMsg();

	if(mFlags & RoutingSendDataFlag_SendReply)
		AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingSendDataOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


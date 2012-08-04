#include "RoutingSendChatOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingSendChatOp::SendRequest()
{
	InitSendMsg(RoutingSendChatRequest);
	mSendMsg.AppendShort(mFlags);
	mSendMsg.AppendWString(mText);
	mSendMsg.AppendShort(mRecipients.size());
	RoutingRecipientList::iterator anItr = mRecipients.begin();
	while(anItr!=mRecipients.end())
	{
		mSendMsg.AppendShort(*anItr);
		++anItr;
	}

	SendMsg();

	if(mFlags & RoutingChatFlag_SendReply)
		AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingSendChatOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


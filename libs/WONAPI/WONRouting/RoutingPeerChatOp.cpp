#include "RoutingPeerChatOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingPeerChatOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingPeerChat)
		return WS_RoutingOp_DontWantReply;


	mSenderId = theMsg.ReadShort();
	mFlags = theMsg.ReadShort();
	theMsg.ReadWString(mText);

	unsigned short aNumRecipients = theMsg.ReadShort();
	for(int i=0; i<aNumRecipients; i++)
		mRecipients.push_back(theMsg.ReadShort());

	return WS_Success;
}


#include "RoutingGroupJoinAttemptOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGroupJoinAttemptOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGroupJoinAttempt)
		return WS_RoutingOp_DontWantReply;

	mClientId = theMsg.ReadShort();
	mGroupId = theMsg.ReadShort();
	theMsg.ReadWString(mComment);
	mJoinGroupFlags = theMsg.ReadByte();

	return WS_Success;
}


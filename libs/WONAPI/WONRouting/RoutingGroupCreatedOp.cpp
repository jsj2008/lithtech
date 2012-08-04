#include "RoutingGroupCreatedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGroupCreatedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGroupCreated)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	theMsg.ReadWString(mGroupName);
	mMaxPlayers = theMsg.ReadShort();
	mGroupFlags = theMsg.ReadLong();
	mAsyncFlags = theMsg.ReadLong();

	return WS_Success;
}


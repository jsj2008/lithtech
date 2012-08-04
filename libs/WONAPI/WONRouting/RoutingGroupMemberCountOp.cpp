#include "RoutingGroupMemberCountOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGroupMemberCountOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGroupMemberCount)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	mMemberCount = theMsg.ReadShort();

	return WS_Success;
}


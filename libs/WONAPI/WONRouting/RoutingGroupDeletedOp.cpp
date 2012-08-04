#include "RoutingGroupDeletedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGroupDeletedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGroupDeleted)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	return WS_Success;
}


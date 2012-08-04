#include "RoutingGroupFlagsChangedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGroupFlagsChangedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGroupFlagsChanged)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	mNewGroupFlags = theMsg.ReadLong();
	mNewAsyncFlags = theMsg.ReadLong();

	return WS_Success;
}


#include "RoutingGroupCaptChangedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGroupCaptainChangedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGroupCaptainChanged)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	mNewCaptainId = theMsg.ReadShort();

	return WS_Success;
}


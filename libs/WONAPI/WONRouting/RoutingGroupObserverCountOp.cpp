#include "RoutingGroupObserverCountOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGroupObserverCountOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGroupObserverCount)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	mObserverCount = theMsg.ReadShort();

	return WS_Success;
}


#include "RoutingClientLeftGroupOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingClientLeftGroupOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingClientLeftGroup)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	mClientId = theMsg.ReadShort();

	return WS_Success;
}


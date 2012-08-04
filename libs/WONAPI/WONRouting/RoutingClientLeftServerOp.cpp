#include "RoutingClientLeftServerOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingClientLeftServerOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingClientLeftServer)
		return WS_RoutingOp_DontWantReply;

	mClientId = theMsg.ReadShort();

	return WS_Success;
}


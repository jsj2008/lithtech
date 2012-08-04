#include "RoutingClientFlagsChangedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingClientFlagsChangedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingClientFlagsChanged)
		return WS_RoutingOp_DontWantReply;

	mClientId = theMsg.ReadShort();
	mNewClientFlags = theMsg.ReadLong();

	return WS_Success;
}


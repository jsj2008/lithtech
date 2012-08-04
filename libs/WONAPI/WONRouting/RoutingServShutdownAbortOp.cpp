#include "RoutingServShutdownAbortOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingServerShutdownAbortedOp::HandleReply(unsigned char theMsgType, ReadBuffer &)
{
	if(theMsgType!=RoutingServerShutdownAborted)
		return WS_RoutingOp_DontWantReply;

	return WS_Success;
}


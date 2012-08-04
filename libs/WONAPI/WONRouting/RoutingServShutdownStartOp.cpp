#include "RoutingServShutdownStartOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingServerShutdownStartedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingServerShutdownStarted)
		return WS_RoutingOp_DontWantReply;

	theMsg.ReadWString(mAlertText);
	mSecondsUntilShutdown = theMsg.ReadLong();
	return WS_Success;
}


#include "RoutingServerAlertOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingServerAlertOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingServerAlert)
		return WS_RoutingOp_DontWantReply;

	theMsg.ReadWString(mAlertText);
	return WS_Success;
}


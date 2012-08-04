#include "RoutingClientJoinedServerOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingClientJoinedServerOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingClientJoinedServer)
		return WS_RoutingOp_DontWantReply;

	mClientId = theMsg.ReadShort();
	theMsg.ReadWString(mClientName);
	mClientFlags = theMsg.ReadLong();

	return WS_Success;
}


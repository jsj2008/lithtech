#include "RoutingGroupMPlyrsChangedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGroupMaxPlayersChangedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGroupMaxPlayersChanged)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	mNewMaxPlayers = theMsg.ReadShort();

	return WS_Success;
}


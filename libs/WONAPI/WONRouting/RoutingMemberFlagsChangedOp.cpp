#include "RoutingMemberFlagsChangedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingMemberFlagsChangedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingMemberFlagsChanged)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	mClientId = theMsg.ReadShort();
	mNewMemberFlags = theMsg.ReadByte();

	return WS_Success;
}


#include "RoutingGroupNameChangedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGroupNameChangedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGroupNameChanged)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	theMsg.ReadWString(mNewGroupName);

	return WS_Success;
}


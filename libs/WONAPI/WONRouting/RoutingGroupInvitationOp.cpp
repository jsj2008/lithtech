#include "RoutingGroupInvitationOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGroupInvitationOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGroupInvitation)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	mCaptainId = theMsg.ReadShort();
	mAmInvited = theMsg.ReadBool();
	theMsg.ReadWString(mComment);

	return WS_Success;
}


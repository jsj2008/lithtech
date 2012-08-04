#include "RoutingYouWereMutedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingYouWereMutedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingYouWereMuted)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	mAmMuted = theMsg.ReadBool();
	theMsg.ReadWString(mMuteComment);
	mMuteTime = theMsg.ReadLong();

	return WS_Success;
}

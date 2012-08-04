#include "RoutingYouWereBannedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingYouWereBannedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingYouWereBanned)
		return WS_RoutingOp_DontWantReply;

	mGroupId = theMsg.ReadShort();
	mAmBanned = theMsg.ReadBool();
	theMsg.ReadWString(mBanComment);
	mBanTime = theMsg.ReadLong();

	return WS_Success;
}

#include "RoutingDataObjectDeletedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingDataObjectDeletedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingDataObjectDeleted)
		return WS_RoutingOp_DontWantReply;

	mLinkId = theMsg.ReadShort();
	theMsg.ReadString(mDataType,1);
	theMsg.ReadWString(mDataName,1);

	return WS_Success;
}

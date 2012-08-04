#include "RoutingDataObjectCreatedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingDataObjectCreatedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingDataObjectCreated)
		return WS_RoutingOp_DontWantReply;

	mLinkId = theMsg.ReadShort();
	theMsg.ReadString(mDataType,1);
	theMsg.ReadWString(mDataName,1);
	mData = theMsg.ReadBuf(2);

	return WS_Success;
}

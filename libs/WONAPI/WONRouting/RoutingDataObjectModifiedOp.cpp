#include "RoutingDataObjectModifiedOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingDataObjectModifiedOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingDataObjectModified)
		return WS_RoutingOp_DontWantReply;

	mLinkId = theMsg.ReadShort();
	theMsg.ReadString(mDataType,1);
	theMsg.ReadWString(mDataName,1);
	mOffset = theMsg.ReadShort();
	mIsInsert = theMsg.ReadBool();
	mData = theMsg.ReadBuf(2);

	return WS_Success;
}

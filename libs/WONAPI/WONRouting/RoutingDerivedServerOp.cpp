#include "RoutingDerivedServerOp.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingDerivedServerOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	mMsgType = theMsgType;
	mRecvMsg = new ByteBuffer(theMsg.data() + theMsg.pos(), theMsg.Available());
	return WS_Success;
}

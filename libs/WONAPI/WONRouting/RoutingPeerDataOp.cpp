#include "RoutingPeerDataOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingPeerDataOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingPeerData)
		return WS_RoutingOp_DontWantReply;


	mSenderId = theMsg.ReadShort();
	mData = theMsg.ReadBuf(2);
	return WS_Success;
}


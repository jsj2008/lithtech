#include "RoutingSetGroupMaxPlayersOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingSetGroupMaxPlayersOp::SendRequest()
{
	InitSendMsg(RoutingSetGroupMaxPlayersRequest);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendShort(mMaxPlayers);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingSetGroupMaxPlayersOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}


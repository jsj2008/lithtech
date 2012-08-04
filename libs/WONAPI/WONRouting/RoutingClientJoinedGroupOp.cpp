#include "RoutingClientJoinedGroupOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingClientJoinedGroupOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingClientJoinedGroup)
		return WS_RoutingOp_DontWantReply;

	unsigned char aFlags = theMsg.ReadByte();

	mGroupId = theMsg.ReadShort();
	mClientId = theMsg.ReadShort();
	mMemberFlags = theMsg.ReadByte();

	if(aFlags & RoutingGroupAsyncFlag_DistributeClientName)  
		theMsg.ReadWString(mClientName);

	if(aFlags & RoutingGroupAsyncFlag_DistributeClientFlags)
	{
		mClientFlags = theMsg.ReadLong();
		mHasClientFlags = true; 
	}

	return WS_Success;
}


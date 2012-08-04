#include "RoutingGetMembersOfGroupOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingGetMembersOfGroupOp::SendRequest()
{
	mMemberMap.clear();

	InitSendMsg(RoutingGetMembersOfGroupRequest);
	mSendMsg.AppendShort(mGroupId);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGetMembersOfGroupOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGetMembersOfGroupReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	unsigned short aGroupId = theMsg.ReadShort();
	if(aGroupId!=mGroupId)
		return WS_RoutingOp_DontWantReply;

	if(aStatus==WS_Success)
	{
		unsigned char aFlags = theMsg.ReadByte();
		mHasClientNames = ((aFlags & RoutingGroupAsyncFlag_DistributeClientName) != 0);
		mHasClientFlags = ((aFlags & RoutingGroupAsyncFlag_DistributeClientFlags) != 0);
		mObserverCount = theMsg.ReadShort();
		unsigned short aNumMembers = theMsg.ReadShort();
		for(int j=0; j<aNumMembers; j++)
		{
			unsigned short aLen = theMsg.ReadShort();
			unsigned long aBeginPos = theMsg.pos();

			RoutingMemberInfoPtr aMemberInfo = GetNewMemberInfo();
			aMemberInfo->mClientId = theMsg.ReadShort();
			aMemberInfo->mFlags = theMsg.ReadByte();
			mMemberMap[aMemberInfo->mClientId] = aMemberInfo;

			if(aFlags!=0)
				aMemberInfo->mClientInfo = GetNewClientInfo();

			if(mHasClientNames)  
				theMsg.ReadWString(aMemberInfo->mClientInfo->mName);

			if(mHasClientFlags)
				aMemberInfo->mClientInfo->mFlags = theMsg.ReadLong();

			theMsg.ReadBytes(aLen - (theMsg.pos() - aBeginPos));
		}			
	}

	return aStatus;
}


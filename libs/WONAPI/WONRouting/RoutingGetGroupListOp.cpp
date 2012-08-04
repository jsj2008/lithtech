#include "RoutingGetGroupListOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingGetGroupListOp::SendRequest()
{
	mGroupMap.clear();
	InitSendMsg(RoutingGetGroupListRequest);
	mSendMsg.AppendShort(mFlags);
	mSendMsg.AppendShort(mParentGroupId);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingMemberInfoPtr RoutingGetGroupListOp::GetNewMemberInfo()
{
	if(mConnection.get())
		return RoutingOp::GetNewMemberInfo();
	else
		return new RoutingMemberInfo;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingGroupInfoPtr RoutingGetGroupListOp::GetNewGroupInfo()
{
	if(mConnection.get())
		return RoutingOp::GetNewGroupInfo();
	else
		return new RoutingGroupInfo;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGetGroupListOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGetGroupListReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	if(aStatus==WS_Success)
	{
		ParseReplyExceptForStatus(theMsg);
	}

	return aStatus;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingGetGroupListOp::ParseReplyExceptForStatus(ReadBuffer &theMsg)
{
	mGroupMap.clear();

	mFlags = theMsg.ReadShort();
	unsigned short aNumGroups = theMsg.ReadShort();
	for(int i=0; i<aNumGroups; i++)
	{
		unsigned short aLen = theMsg.ReadShort();
		unsigned long aBeginPos = theMsg.pos();

		RoutingGroupInfoPtr anInfo = GetNewGroupInfo();
		anInfo->mId = theMsg.ReadShort();
		if (HasGroupName())
			theMsg.ReadWString(anInfo->mName);
		if (HasCaptainId())
			anInfo->mCaptainId = theMsg.ReadShort();
		if (HasMaxPlayers())
			anInfo->mMaxPlayers = theMsg.ReadShort();
		if (HasGroupFlags())
			anInfo->mFlags = theMsg.ReadLong();
		if (HasAsyncFlags())
			anInfo->mAsyncFlags = theMsg.ReadLong();
		if (HasObserverCount())
			anInfo->mObserverCount = theMsg.ReadShort();
		if (HasMemberCount())
			anInfo->mMemberCount = theMsg.ReadShort();
	
		if (HasMembers())
		{
			for(int j=0; j<anInfo->mMemberCount; j++)
			{
				RoutingMemberInfoPtr aMemberInfo = GetNewMemberInfo();
				aMemberInfo->mClientId = theMsg.ReadShort();
				aMemberInfo->mFlags = theMsg.ReadByte();
				anInfo->mMemberMap[aMemberInfo->mClientId] = aMemberInfo;
			}
		}

	//	if (theMsg.HasMoreBytes())
	//		anInfo->mParentId = theMsg.ReadShort();

		mGroupMap[anInfo->mId] = anInfo;
		
		theMsg.ReadBytes(aLen - (theMsg.pos() - aBeginPos));
	}

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RoutingGetGroupListOp::ParseGroupListObj(const void *theData, unsigned long theDataLen)
{
	try
	{
		ReadBuffer aMsg(theData,theDataLen);
		ParseReplyExceptForStatus(aMsg);
		return true;
	}
	catch(ReadBufferException&)
	{
	}
	return false;
}

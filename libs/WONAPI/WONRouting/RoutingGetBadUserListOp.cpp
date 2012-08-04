#include "RoutingGetBadUserListOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingGetBadUserListOp::SendRequest()
{
	InitSendMsg(RoutingGetBadUserListRequest);
	mSendMsg.AppendByte(mListType);
	mSendMsg.AppendShort(mGroupId);
	mSendMsg.AppendByte(mFlags);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGetBadUserListOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGetBadUserListReply)
		return WS_RoutingOp_DontWantReply;

	mBadUserList.clear();

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	if(aStatus==WS_Success)
	{
		mListType = (RoutingBadUserListType)theMsg.ReadByte();
		mGroupId = theMsg.ReadShort();
		mFlags = theMsg.ReadByte();
		unsigned short aNumUsers = theMsg.ReadShort();

		for(int i=0; i<aNumUsers; i++)
		{
			unsigned short aLen = theMsg.ReadShort();
			unsigned long aBeginPos = theMsg.pos();

			RoutingBadUser aBadUser;
			aBadUser.mWONId = theMsg.ReadLong();
			theMsg.ReadWString(aBadUser.mName);
			if (HasExpirationDiff())
				aBadUser.mExpirationDiff = theMsg.ReadLong();
			if (HasModeratorWONUserId())
				aBadUser.mModeratorWONId = theMsg.ReadLong();
			if (HasModeratorName())
				theMsg.ReadWString(aBadUser.mModeratorName);
			if (HasModeratorComment())
				theMsg.ReadWString(aBadUser.mModeratorComment);

			mBadUserList.push_back(aBadUser);

			theMsg.ReadBytes(aLen - (theMsg.pos() - aBeginPos));
		}
	}

	return aStatus;
}


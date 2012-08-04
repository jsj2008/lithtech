#include "RoutingRegisterClientOp.h"
#include "RoutingGetClientListOp.h"
#include "RoutingGetGroupListOp.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingRegisterClientOp::RoutingRegisterClientOp(RoutingConnection *theConnection) : RoutingOp(theConnection)
{
	mReconnectId = 0;
	mAsyncMessageFlags = 0;
	mRegisterFlags = 0;
	mClientFlags = 0;
	mClientId = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingRegisterClientOp::RoutingRegisterClientOp(RoutingConnection *theConnection, const std::wstring &theClientName,
								   const std::wstring &theServerPassword, unsigned long theReconnectId, 
								   unsigned long theAsyncMessageFlags, unsigned long theRegisterFlags,
								   unsigned long theClientFlags) :
	RoutingOp(theConnection),
	mClientName(theClientName),
	mServerPassword(theServerPassword),
	mReconnectId(theReconnectId),
	mAsyncMessageFlags(theAsyncMessageFlags),
	mRegisterFlags(theRegisterFlags),
	mClientFlags(theClientFlags)
{
	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingRegisterClientOp::SendRequest()
{
	mReplyCount = 0;
	mNumRepliesNeeded = 1;
	if(mRegisterFlags & RoutingRegisterClientFlag_GetClientList)
		mNumRepliesNeeded++;

	InitSendMsg(RoutingRegisterClientRequest);	
	mSendMsg.AppendWString(mClientName);
	mSendMsg.AppendWString(mServerPassword);
	mSendMsg.AppendLong(mReconnectId);
	mSendMsg.AppendLong(mAsyncMessageFlags);
	mSendMsg.AppendLong(mRegisterFlags);
	mSendMsg.AppendLong(mClientFlags);

	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingRegisterClientOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	WONStatus aStatus;

	if(theMsgType==RoutingGetClientListReply && (mRegisterFlags&RoutingRegisterClientFlag_GetClientList))
	{
		RoutingGetClientListOpPtr anOp = new RoutingGetClientListOp(mConnection);
		aStatus = anOp->HandleReply(theMsgType,theMsg);
		mClientMap = anOp->GetClientMap();
		mReplyCount++;
	}
	else if(theMsgType==RoutingRegisterClientReply)
	{
		aStatus = (WONStatus)theMsg.ReadShort();
		if(aStatus==WS_Success)
		{
			mClientId = theMsg.ReadShort();
			theMsg.ReadWString(mClientName);
			mReconnectId = theMsg.ReadLong();
		}
		mReplyCount++;
	}
	else
		return WS_RoutingOp_DontWantReply;

	if(aStatus!=WS_Success || mReplyCount==mNumRepliesNeeded)
		return aStatus;
	else
		return WS_RoutingOp_NeedMoreReplies;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


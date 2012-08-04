#include "RoutingDetectFirewallOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingDetectFirewallOp::RoutingDetectFirewallOp(RoutingConnection *theConnection) :
	RoutingOp(theConnection),
	mListenPort(0),
	mMaxConnectWaitTime(0),
	mDoListen(true),
	mWaitingForStatusReply(false)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingDetectFirewallOp::CleanupHook()
{
	RoutingOp::CleanupHook();
	if(mListenSocket.get()!=NULL)
	{
		mListenSocket->Close();
		mListenSocket = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingDetectFirewallOp::SendRequest()
{
	InitSendMsg(RoutingDetectFirewallRequest);
	if(mDoListen)
	{
		mListenSocket = new BlockingSocket;
		WONStatus aStatus = mListenSocket->Bind(mListenPort);
		if(aStatus!=WS_Success)
		{
			Finish(aStatus);
			return;
		}

		aStatus = mListenSocket->Listen();
		if(aStatus!=WS_Success)
		{
			Finish(aStatus);
			return;
		}

		mListenPort = mListenSocket->GetLocalPort();
	}


	mWaitingForStatusReply = true;
	mSendMsg.AppendShort(mListenPort);
	mSendMsg.AppendLong(mMaxConnectWaitTime);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingDetectFirewallOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	WONStatus aStatus;
	if(mWaitingForStatusReply && theMsgType==RoutingStatusReply)
	{
		mWaitingForStatusReply = false;
		aStatus = (WONStatus)theMsg.ReadShort();
		if(aStatus!=WS_Success)
			return aStatus;
		else
			return WS_RoutingOp_NeedMoreReplies;
	}
	else if(theMsgType==RoutingDetectFirewallResult)
	{
		aStatus = (WONStatus)theMsg.ReadShort();
		mListenPort = theMsg.ReadShort();
		return aStatus;
	}
	else
		return WS_RoutingOp_DontWantReply;
}

#include "DetectFirewallOp.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void DetectFirewallOp::Init()
{
	mLengthFieldSize	= 2;
	mListenPort			= 0;
	mMaxConnectWaitTime = 0;
	mDoListen			= true;
	mUseUDP				= false;

	SetMaxRecvTime(OP_TIMEOUT_INFINITE);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DetectFirewallOp::DetectFirewallOp(ServerContext *theFirewallDetectorContext) : ServerRequestOp(theFirewallDetectorContext)
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DetectFirewallOp::DetectFirewallOp(const IPAddr &theAddr) : ServerRequestOp(theAddr)
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void DetectFirewallOp::RunHook()
{
	if(mDoListen || mUseUDP)
	{
		mListenSocket = new BlockingSocket(mUseUDP ? AsyncSocket::UDP : AsyncSocket::TCP);
		WONStatus aStatus = mListenSocket->Bind(mListenPort);
		if(aStatus!=WS_Success)
		{
			Finish(aStatus);
			return;
		}

		if (!mUseUDP)
		{
			// Listen for a connection
			aStatus = mListenSocket->Listen();
			if(aStatus!=WS_Success)
			{
				Finish(aStatus);
				return;
			}
		}

		mListenPort = mListenSocket->GetLocalPort();
	}

	ServerRequestOp::RunHook();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool DetectFirewallOp::CallbackHook(AsyncOp *theOp, int theParam)
{
	if(theParam!=DetectFirewall_Track_RecvFrom)
		return ServerRequestOp::CallbackHook(theOp, theParam);

	if (theOp->Succeeded())
	{
		// Ignore packets from unknown addresses
		if (mRecvBytesFromOp->GetAddr().GetHost() != GetCurAddr().GetHost())
		{
			// Receive again
			SetupReceiveTracker();
			return true;
		}
	}


	Finish(theOp->GetStatus());
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void DetectFirewallOp::CleanupHook()
{
	ServerRequestOp::CleanupHook();
	if(mListenSocket.get()!=NULL)
	{
		mListenSocket->Close();
		mListenSocket = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus DetectFirewallOp::GetNextRequest()
{
	WriteBuffer aBuf(mLengthFieldSize);
	aBuf.AppendByte(5);			// small message
	aBuf.AppendShort(3);		// FirewallDetector Service
	aBuf.AppendShort( mUseUDP ? 3 : 1 );	// Detect Firewall Type (1 = TCP, 3 = UDP)
	aBuf.AppendShort(mListenPort);

	if (!mUseUDP)
	{
		aBuf.AppendBool(true); // wait for connect
		aBuf.AppendLong(mMaxConnectWaitTime);
		aBuf.AppendBool(false); // send reply to acceptor
	}

	mRequest = aBuf.ToByteBuffer();
	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus DetectFirewallOp::CheckResponse()
{
	ReadBuffer aMsg(mResponse->data(),mResponse->length());
	unsigned char aHeaderType = aMsg.ReadByte();
	unsigned short aServiceType = aMsg.ReadShort();
	unsigned short aMessageType = aMsg.ReadShort();

	if(aHeaderType!=5 || aServiceType!=3 || aMessageType!=2)
			return InvalidReplyHeader();


	if (mUseUDP)
		return SetupReceiveTracker();
	else
	{
		short aStatus = aMsg.ReadShort();
		return (WONStatus)aStatus;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus DetectFirewallOp::SetupReceiveTracker()
{
	// Wait to receive bytes
	if(IsAsync())
	{
		mRecvBytesFromOp = new RecvBytesFromOp(255, mListenSocket);
		DWORD aWaitTime = TimeLeft();
		if(mMaxConnectWaitTime > 0 && mMaxConnectWaitTime < aWaitTime)
			aWaitTime = mMaxConnectWaitTime;

		Track(mRecvBytesFromOp,DetectFirewall_Track_RecvFrom);
		if(aWaitTime == 0)
			aWaitTime = OP_TIMEOUT_INFINITE; 

		mRecvBytesFromOp->RunAsync(aWaitTime);
		return WS_ServerReq_ExitCommunicationLoop;
	}

	// Blocking
	do
	{
		mRecvBytesFromOp = new RecvBytesFromOp(255, mListenSocket);
		DWORD aWaitTime = TimeLeft();
		if(mMaxConnectWaitTime > 0 && mMaxConnectWaitTime < aWaitTime)
			aWaitTime = mMaxConnectWaitTime;

		WONStatus result = mRecvBytesFromOp->Run(OP_MODE_BLOCK, aWaitTime);
		if (result != WS_Success)
			return result;

	} // Ignore packets from unknown addresses
	while (mRecvBytesFromOp->GetAddr().GetHost() != GetCurAddr().GetHost());

	
	// Packet received from the correct address
	return WS_Success;
}
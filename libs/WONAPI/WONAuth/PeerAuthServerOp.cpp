#include "PeerAuthServerOp.h"
#include "GetCertOp.h"
#include "WONCommon/WriteBuffer.h"
#include "WONSocket/SocketOp.h"
#include "CryptTransform.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
PeerAuthServerOp::PeerAuthServerOp(AuthContext *theAuthContext, BlockingSocket *theSocket)
{
	mAuthContext = theAuthContext;
	mSocket = theSocket;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
PeerAuthServerOp::~PeerAuthServerOp()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool PeerAuthServerOp::CallbackHook(AsyncOp *theOp, int)
{
	if(!theOp->Succeeded())
	{
		Finish(theOp->GetStatus());
		return true;
	}

	RecvMsgOp *aRecvOp = dynamic_cast<RecvMsgOp*>(theOp);
	if(aRecvOp==NULL)
		return true;

	ByteBufferPtr aSendMsg;
	ByteBufferPtr aRecvMsg = aRecvOp->GetMsg();
	WONStatus aStatus = mPeerAuthServer.HandleRecvMsg(aRecvMsg->data(),aRecvMsg->length(),aSendMsg);
	if(aSendMsg.get()!=NULL)
		mSocket->QueueOp((SocketOp*)Track(new SendMsgOp(aSendMsg)), SendTime()); // send reply

	if(aStatus!=WS_Success)
		Finish(aStatus);
	else if(mPeerAuthServer.GetState()==PeerAuthServer::STATE_NOT_STARTED) // --> finished
		Success();
	else
		mSocket->QueueOp((SocketOp*)Track(new RecvMsgOp), RecvTime()); // get ready for challenge2

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool PeerAuthServerOp::CheckStatus(WONStatus theStatus)
{
	if(theStatus==WS_Success)
		return false;
	else
	{
		Finish(theStatus);
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthServerOp::AsyncRun()
{
	// Get ready for the auth request
	mSocket->QueueOp((SocketOp*)Track(new RecvMsgOp), RecvTime());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthServerOp::RunHook()
{	
	mLengthFieldSize = mSocket->GetLengthFieldSize();
	mPeerAuthServer.Start(mAuthContext->GetPeerData(), mLengthFieldSize);

	// Asynchronous operation
	if(IsAsync())
	{
		AsyncRun();
		return;
	}

	// Blocking operation
	WONStatus aStatus;

	// Receive AuthRequest
	ByteBufferPtr aRecvMsg;
	aStatus = mSocket->RecvMsg(aRecvMsg, RecvTime());
	if(CheckStatus(aStatus))
		return;

	ByteBufferPtr aSendMsg;
	aStatus = mPeerAuthServer.HandleRecvMsg(aRecvMsg->data(), aRecvMsg->length(), aSendMsg);
	CheckStatus(aStatus);
	if(aSendMsg.get()==NULL)
		return;

	// Send Challenge1 (or Complete if error)
	aStatus = mSocket->SendMsg(aSendMsg, SendTime());
	CheckStatus(aStatus);

	if(!Pending())
		return;


	// Receive Challenge2
	aStatus = mSocket->RecvMsg(aRecvMsg, RecvTime());
	if(CheckStatus(aStatus))
		return;
	
	aStatus = mPeerAuthServer.HandleRecvMsg(aRecvMsg->data(), aRecvMsg->length(), aSendMsg);
	CheckStatus(aStatus);
	if(aSendMsg.get()==NULL)
		return;

	// Send Complete
	aStatus = mSocket->SendMsg(aSendMsg, SendTime());
	if(aStatus==WS_Success)
		Success();
	else
		Finish(aStatus);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthServerOp::Success()
{
	AuthSession *aSession = GetSession();
	AuthType aType = aSession->GetAuthType();
	if(aType==AUTH_TYPE_SESSION || aType==AUTH_TYPE_PERSISTENT) // add crypt protocol
		mSocket->PushMsgTransform(new CryptTransform(aSession));

	Finish(WS_Success);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthServerOp::CleanupHook()
{
	AsyncOpWithTracker::CleanupHook();	
}

#include "WONCommon/ReadBuffer.h"
#include "WONCommon/WriteBuffer.h"
#include "WONSocket/SocketOp.h"
#include "WONAuth/CryptTransform.h"
#include "WONAuth/PeerAuthOp.h"
#include "ServerRequestOp.h"

#include <algorithm>
using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ServerRequestOp::Init()
{
	mLengthFieldSize = 4;
	mAuthType = AUTH_TYPE_NONE;
	mMaxConnectTime = 20000;
	mMaxSendTime = 20000;
	mMaxRecvTime = 20000;

	mGetCertStatus = WS_None;
	mLastServerError = WS_None;

	mUseAuth2 = true;
	mUseLastServerErrorForFinishStatus = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ServerRequestOp::ServerRequestOp(ServerContext *theContext)
{
	Init();
	SetServerContext(theContext);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ServerRequestOp::ServerRequestOp(const IPAddr &theAddr)
{
	Init();
	SetAddr(theAddr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ServerRequestOp::SetServerContext(ServerContext *theContext)
{
	mServerContext = theContext;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ServerRequestOp::SetAddr(const IPAddr &theAddr)
{
	mServerContext = NULL;
	mAddrList.clear();
	mAddrList.push_back(theAddr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
IPAddr ServerRequestOp::GetAddr() const
{
	if(mAddrList.empty())
		return IPAddr();
	else
		return mAddrList.front();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ServerRequestOp::FinishSaveSession(WONStatus theStatus)
{
	Finish(theStatus);
	if(mAuthType==AUTH_TYPE_SESSION)
		mAuthContext->PutSession(mCurAddr,mCurSession);

	mCurSession = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ServerRequestOp::SetLastServerError(WONStatus theError)
{
	if(mLastServerError==WS_None)
		mUseLastServerErrorForFinishStatus = true;
	else if(theError!=mLastServerError)
		mUseLastServerErrorForFinishStatus = false;

	mLastServerError = theError;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus ServerRequestOp::InvalidReplyHeader()
{
	SetLastServerError(WS_ServerReq_InvalidReplyHeader);
	return WS_ServerReq_TryNextServer;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ServerRequestOp::TryConnect()
{
	if(mCurSession.get()!=NULL)
	{
		mSocket->PushMsgTransform(new CryptTransform(mCurSession));
		mReusingSession = true;
	}

	if(IsAsync())
	{	
		QueueSocketOp(new ConnectOp(mCurAddr),ConnectTime());
		Send();
		return true;
	}
	else
	{
		WONStatus aStatus = mSocket->Connect(mCurAddr,ConnectTime());
		if(aStatus==WS_Success)
			return true;
		else
			SetLastServerError(aStatus);
	}

	return false;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ServerRequestOp::TryPeerToPeerAuth()
{
	PeerAuthOpPtr aPeerAuthOp = new PeerAuthOp(mCurAddr,mAuthContext,mAuthType,mSocket,mUseAuth2);
	aPeerAuthOp->CopyMaxTimes(this);
	if(IsAsync())
	{
		Track(aPeerAuthOp,ServerReq_Track_PeerAuth);
		aPeerAuthOp->RunAsync(OP_TIMEOUT_INFINITE);
		return true;
	}
	else
	{
		if(aPeerAuthOp->RunBlock(TimeLeft()))
		{
			mCurSession = aPeerAuthOp->GetSession();
			return true;
		}
		else if(aPeerAuthOp->GetStatus()==WS_PeerAuth_GetCertFailure)
		{
			mGetCertStatus = aPeerAuthOp->GetGetCertStatus();
			Finish(WS_ServerReq_GetCertFailure);
		}
		else
			SetLastServerError(aPeerAuthOp->GetStatus());
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ServerRequestOp::QueueSocketOp(SocketOp *theOp, DWORD theTimeout)
{
	mSocket->QueueOp((SocketOp*)Track(theOp,ServerReq_Track_Socket),theTimeout);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ServerRequestOp::CallbackHook(AsyncOp *theOp, int theParam)
{
	if(theOp->Killed()) 
	{
		Kill();
		return true;
	}

	if(!theOp->Succeeded())
	{
		if(theParam==ServerReq_Track_PeerAuth && theOp->GetStatus()==WS_PeerAuth_GetCertFailure)
		{
			mGetCertStatus = ((PeerAuthOp*)theOp)->GetGetCertStatus();
			Finish(WS_ServerReq_GetCertFailure);
			return true;
		}

		SetLastServerError(theOp->GetStatus());
		TryNextServer();
		return true;
	}

	switch(theParam) 
	{
		case ServerReq_Track_PeerAuth:
		{
			PeerAuthOp *aPeerAuthOp = (PeerAuthOp*)theOp;
			mCurSession = aPeerAuthOp->GetSession();
			Send();
		}
		
		break;

		case ServerReq_Track_Socket:
		{
			RecvMsgOp *aRecvOp = dynamic_cast<RecvMsgOp*>(theOp);
			if(aRecvOp==NULL)
				break;

			mResponse = aRecvOp->GetMsg();
			if(CheckExpiredSession())
			{
				TryNextServer();
				break;
			}

			try
			{
				WONStatus aStatus = CheckResponse();
				if(aStatus==WS_ServerReq_TryNextServer) // error, try next server
					TryNextServer();
				else if(aStatus==WS_ServerReq_Send) // more data to send
					Send();
				else if(aStatus==WS_ServerReq_Recv) // more data to receive
					QueueSocketOp(new RecvMsgOp, RecvTime());
				else if(aStatus==WS_ServerReq_ExitCommunicationLoop)
					return true;
				else // done
					FinishSaveSession(aStatus);
			}
			catch(ReadBufferException&)
			{
				SetLastServerError(WS_ServerReq_UnpackFailure);
				TryNextServer();
			}	
		}
		break;

		default:
			return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ServerRequestOp::TryNextServer()
{
	if(!Pending())
		return false;


	while(true)
	{	
		mOpTracker.KillAll();

		Reset();
		if(mSocket.get()!=NULL)
		{
			mSocket->Close();
			mSocket = NULL;
		}

		if(mAddrItr!=mAddrList.begin() && !mSessionExpired)
		{
			if(mServerContext.get()!=NULL)
				mServerContext->NotifyFailed(mCurAddr);
		}

		if(mAddrItr==mAddrList.end()) // No more servers
		{
			if(mUseLastServerErrorForFinishStatus)
				Finish(mLastServerError);
			else
				Finish(WS_ServerReq_FailedAllServers);
			
			return false;
		}

		mSocket = new BlockingSocket;
		mSocket->SetLengthFieldSize(mLengthFieldSize);

		mCurAddr = *mAddrItr;
		bool wasReusingSession = mReusingSession;
		mReusingSession = false;
		mSessionExpired = false;
		++mAddrItr;

		// If we didn't already fail at reusing a session then let's try to use one
		// if we're supposed to be performing sessioned communication
		if(!wasReusingSession && mAuthType==AUTH_TYPE_SESSION)
			mCurSession = mAuthContext->GetSession(mCurAddr);
		else
			mCurSession = NULL;
		
		if(mCurSession.get()!=NULL || mAuthType==AUTH_TYPE_NONE)  // Just need to connect
		{
			if(TryConnect()) 
				return true;
		}
		else // Need Peer to Peer Authentication
		{
			if(TryPeerToPeerAuth())
				return true;
			else if(!Pending()) // may have finished due to GetCertFailure
				return false;
		}

		if(TimeLeft()<=0)
		{
			Finish(WS_TimedOut);
			return false;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

WONStatus ServerRequestOp::Send()
{
	while(true)
	{
		WONStatus aReqStatus = GetNextRequest();
		if(aReqStatus!=WS_ServerReq_Send && aReqStatus!=WS_ServerReq_Recv)
			return aReqStatus;

		if(IsAsync())
			QueueSocketOp(new SendMsgOp(mRequest),SendTime());
		else
		{
			WONStatus aSendStatus = mSocket->SendMsg(mRequest, SendTime());
			if(aSendStatus!=WS_Success)
			{
				SetLastServerError(aSendStatus);
				return WS_ServerReq_TryNextServer;
			}
		}

		if(aReqStatus==WS_ServerReq_Recv) // done sending
		{
			if(IsAsync()) // Get recv ready
				QueueSocketOp(new RecvMsgOp,RecvTime());

			return aReqStatus;
		}
	} 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ServerRequestOp::CheckExpiredSession()
{
	if(!mReusingSession) // can't have expired session if not reusing session
		return false;

	if(mResponse->length()<10)
		return false;
	try
	{
		ReadBuffer aBuf(mResponse->data(),mResponse->length());
		DWORD aServiceType = aBuf.ReadLong();
		DWORD aMessageType = aBuf.ReadLong();
		if(aServiceType==1 && aMessageType==15) // common status reply --> session problem
		{
			// try server again, but perform full peer to peer authentication
			mSessionExpired = true;
			mAddrItr--;

			return true;
		}
	}
	catch(ReadBufferException&)
	{
	}
	
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus ServerRequestOp::Recv()
{
	while(true)
	{
		WONStatus aStatus = mSocket->RecvMsg(mResponse,RecvTime());
		if(aStatus!=WS_Success)
		{
			SetLastServerError(aStatus);
			return WS_ServerReq_TryNextServer;
		}
		
		if(CheckExpiredSession())
			return WS_ServerReq_SessionExpired;

		try
		{
			aStatus = CheckResponse();
			if(aStatus!=WS_ServerReq_Recv)
				return aStatus;
		}
		catch(ReadBufferException&)
		{
			SetLastServerError(WS_ServerReq_UnpackFailure);
			return WS_ServerReq_TryNextServer;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ServerRequestOp::RunHook()
{
	mGetCertStatus = WS_None;
	mLastServerError = WS_None;
	mUseLastServerErrorForFinishStatus = false;

	// Get list of servers
	if(mServerContext.get()!=NULL)
		mServerContext->CopyAddresses(mAddrList);

	if(mAddrList.empty())
	{
		Finish(WS_ServerReq_NoServersSpecified);
		return;
	}

	mAddrItr = mAddrList.begin();
	mReusingSession = false;
	mSessionExpired = false;

	// Setup authentication
	if(mAuthType!=AUTH_TYPE_NONE && mAuthContext.get()==NULL)
	{
		// need auth context to authenticate
		Finish(WS_ServerReq_NeedAuthContext);
		return;
	}

	// If asynchronous then just start the process
	if(IsAsync())
	{
		TryNextServer();
		return;
	}


	WONStatus aStatus = WS_None;

	// If synchronous then do it all right here
	while(TryNextServer()) // Try each server
	{
		while(true) // Talk to server
		{
			// Send requests
			do
			{ 
				aStatus = Send();
			} while(aStatus==WS_ServerReq_Send);

			if(aStatus==WS_ServerReq_TryNextServer)
				break;

			// Receive and validate replies
			do
			{
				aStatus = Recv();
			} while(aStatus==WS_ServerReq_Recv);

			if(aStatus==WS_ServerReq_TryNextServer)
				break;
			else if(aStatus==WS_ServerReq_SessionExpired)
				break;
			else if(aStatus==WS_ServerReq_Send)
				continue;
			else 
			{
				FinishSaveSession(aStatus);
				return;
			}
		}

		if(TimeLeft()<=0)
		{
			Finish(WS_TimedOut);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ServerRequestOp::CleanupHook()
{
	AsyncOpWithTracker::CleanupHook();

	if(TimedOut() && mServerContext.get()!=NULL)
		mServerContext->NotifyFailed(mCurAddr);

	if(mSocket.get()!=NULL)
	{
		mSocket->Close();
		mSocket = NULL;
	}

	mCurSession = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

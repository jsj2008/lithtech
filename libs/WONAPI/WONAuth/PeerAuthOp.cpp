#include "PeerAuthOp.h"
#include "GetCertOp.h"
#include "WONCommon/WriteBuffer.h"
#include "WONSocket/SocketOp.h"
#include "CryptTransform.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
PeerAuthOp::PeerAuthOp(const IPAddr &theAddr, AuthContext *theAuthContext, AuthType theType, 
					   BlockingSocket *theSocket, bool useAuth2) : mConnectAddr(theAddr)
{
	mAuthContext = theAuthContext;
	mAuthType = theType;
	mSocket = theSocket;
	mPeerAuthClient.SetUseAuth2(useAuth2);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
PeerAuthOp::~PeerAuthOp()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool PeerAuthOp::CallbackHook(AsyncOp *theOp, int theParam)
{
	switch(theParam)
	{
		case PeerAuth_Track_Socket:
		{
			if(!theOp->Succeeded())
			{
				Finish(theOp->GetStatus());
				break;
			}

			if(mAuthType==AUTH_TYPE_NONE) // just connecting
			{
				Finish(WS_Success);
				break;
			}

			RecvMsgOp *aRecvOp = dynamic_cast<RecvMsgOp*>(theOp);
			if(aRecvOp==NULL)
				break;

			ByteBufferPtr aSendMsg;
			ByteBufferPtr aRecvMsg = aRecvOp->GetMsg();
			WONStatus aStatus = mPeerAuthClient.HandleRecvMsg(aRecvMsg->data(),aRecvMsg->length(),aSendMsg);
			if(aStatus==WS_CommServ_InvalidParameters || aStatus==WS_CommServ_ExpiredPeerCertificate) // expired certificate
			{
				mPeerAuthClient.Reset();
				switch(mGetCertStatus)
				{
					case WS_None: AsyncRefreshCert(); break;
					case WS_Pending: break;
					case WS_Success: AsyncRetry(); break;
					default: Finish(WS_PeerAuth_GetCertFailure);
				}	

				break;
			}
			else if(CheckStatus(aStatus))
				break;

			if(aSendMsg.get()==NULL) 
				Success(); 				// done
			else
				AsyncSend(aSendMsg); 	// Need to send more stuff
		}
		break;


		case PeerAuth_Track_GetCert:
		{
			LightGetCertOp *aGetCert = (LightGetCertOp*)theOp;
			mGetCertStatus = aGetCert->GetStatus();
			mPeerData = aGetCert->GetPeerData();
			if(mPeerAuthClient.GetState()==PeerAuthClient::STATE_NOT_STARTED)
			{
				if(mGetCertStatus!=WS_Success)
				{
					Finish(WS_PeerAuth_GetCertFailure);
					break;
				}
				else
					AsyncRetry();
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
bool PeerAuthOp::CheckStatus(WONStatus theStatus)
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
void PeerAuthOp::AsyncRetry()
{
	mGetCertStatus = WS_CommServ_InvalidParameters; 
	AsyncConnect();
	AsyncSend(mPeerAuthClient.Start(mPeerData,mAuthType,mLengthFieldSize));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthOp::AsyncSend(const ByteBuffer *theMsg)
{
	mSocket->QueueOp((SocketOp*)Track(new SendMsgOp(theMsg),PeerAuth_Track_Socket), SendTime());
	mSocket->QueueOp((SocketOp*)Track(new RecvMsgOp,PeerAuth_Track_Socket), RecvTime());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthOp::AsyncConnect()
{
	mSocket->QueueOp((SocketOp*)Track(new ConnectOp(mConnectAddr),PeerAuth_Track_Socket), ConnectTime());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthOp::AsyncRefreshCert()
{
	mGetCertStatus = WS_Pending;
	LightGetCertOpPtr aGetCert = new LightGetCertOp;
	Track(aGetCert,PeerAuth_Track_GetCert);
	mAuthContext->RefreshAsync(aGetCert);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthOp::AsyncRun()
{
	mGetCertStatus = WS_None;
	if(mAuthType!=AUTH_TYPE_NONE)
	{
		if(mPeerData->CertificateExpired(120))
			AsyncRefreshCert();

		if(mPeerData->CertificateExpired(30)) // don't even try until get the certificate
			return;
	}

	AsyncConnect();
	if(mAuthType!=AUTH_TYPE_NONE)
		AsyncSend(mPeerAuthClient.Start(mPeerData,mAuthType,mLengthFieldSize));		
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthOp::RunHook()
{	
	mLengthFieldSize = mSocket->GetLengthFieldSize();

	if(mAuthType!=AUTH_TYPE_NONE)
		mPeerData = mAuthContext->GetPeerData();

	// Asynchronous operation
	if(IsAsync())
	{
		AsyncRun();
		return;
	}

	// Blocking operation

	// Refresh certificate if necessary
	if(mAuthType!=AUTH_TYPE_NONE && mPeerData->CertificateExpired(30))
	{
		mGetCertStatus = mAuthContext->RefreshBlock(TimeLeft());
		if(mGetCertStatus!=WS_Success)
		{
			Finish(WS_PeerAuth_GetCertFailure);
			return;
		}

		mPeerData = mAuthContext->GetPeerData();
	}

	for(int i=0; i<2; i++)
	{
		WONStatus aStatus;

		// Connect
		aStatus = mSocket->Connect(mConnectAddr, ConnectTime());
		if(CheckStatus(aStatus))
			return;

		// If no auth then done
		if(mAuthType==AUTH_TYPE_NONE)
		{
			Finish(WS_Success);
			return;
		}

		// Send peer auth request
		ByteBufferPtr aSendMsg = mPeerAuthClient.Start(mPeerData,mAuthType,mLengthFieldSize);
		aStatus = mSocket->SendMsg(aSendMsg, SendTime());
		if(CheckStatus(aStatus))
			return;

		// Receive challenge1
		ByteBufferPtr aRecvMsg;
		aStatus = mSocket->RecvMsg(aRecvMsg, RecvTime());
		if(CheckStatus(aStatus))
			return;

		// Check challenge 1
		aStatus = mPeerAuthClient.HandleRecvMsg(aRecvMsg->data(),aRecvMsg->length(),aSendMsg);
		if(i==0 && (aStatus==WS_CommServ_InvalidParameters || aStatus==WS_CommServ_ExpiredPeerCertificate)) // expired certificate
		{
			mPeerAuthClient.Reset();
			mGetCertStatus = mAuthContext->RefreshBlock(TimeLeft());
			if(mGetCertStatus!=WS_Success)
			{
				Finish(WS_PeerAuth_GetCertFailure);
				return;
			}

			mPeerData = mAuthContext->GetPeerData();
			continue; // try one more time
		}
		else if(CheckStatus(aStatus))
			return;


		// Send challenge 2 
		aStatus = mSocket->SendMsg(aSendMsg, SendTime());
		if(CheckStatus(aStatus))
			return;

		// Receive peer auth complete
		aStatus = mSocket->RecvMsg(aRecvMsg, RecvTime());
		if(CheckStatus(aStatus))
			return;

		// Check peer auth complete
		aStatus = mPeerAuthClient.HandleRecvMsg(aRecvMsg->data(),aRecvMsg->length(),aSendMsg);
		if(CheckStatus(aStatus))
			return;

		// Success... add encryption protocol if necessary
		Success();
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthOp::Success()
{
	if(mAuthType==AUTH_TYPE_SESSION || mAuthType==AUTH_TYPE_PERSISTENT) // add crypt protocol
		mSocket->PushMsgTransform(new CryptTransform(GetSession()));

	Finish(WS_Success);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthOp::CleanupHook()
{
	AsyncOpWithTracker::CleanupHook();	
//	mSocket = NULL;
}

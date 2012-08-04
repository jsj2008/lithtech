#include "PeerAuthServer.h"
#include "WONCommon/WriteBuffer.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
PeerAuthServer::PeerAuthServer()
{
	mState = STATE_NOT_STARTED;
	mUseAuth2 = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthServer::Start(const AuthPeerData *theData, unsigned char theLengthFieldSize)
{
	mPeerData = theData;
	mLengthFieldSize = theLengthFieldSize;
	mState = STATE_AWAITING_REQUEST;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus PeerAuthServer::GetChallenge1(ByteBufferPtr &theChallenge)
{
	WriteBuffer aChallenge(mLengthFieldSize);

	aChallenge.AppendLong(203);					// Auth peer to peer service
	aChallenge.AppendLong(51);					// Challenge1

	mSecretB.Create(8);
	WriteBuffer aChallengeSecret;
	aChallengeSecret.AppendShort(mSecretB.GetKeyLen());
	aChallengeSecret.AppendBytes(mSecretB.GetKey(),mSecretB.GetKeyLen());

	ByteBufferPtr anEncrypt = mClientCertificate->GetPubKey().Encrypt(aChallengeSecret.data(),aChallengeSecret.length());
	if(anEncrypt.get()==NULL)
		return WS_PeerAuthServer_FailedToEncryptWithClientPubKey;

	aChallenge.AppendShort(anEncrypt->length());
	aChallenge.AppendBytes(anEncrypt->data(),anEncrypt->length());

	if(mUseAuth2)
		aChallenge.AppendBuffer(mPeerData->GetCertificate2()->GetRawBuf(),2);
	else
		aChallenge.AppendBuffer(mPeerData->GetCertificate()->GetRawBuf(),2);

	theChallenge = aChallenge.ToByteBuffer();
	return WS_Success;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus PeerAuthServer::HandleRequest(ReadBuffer &theRequest, ByteBufferPtr &theChallenge)
{
	unsigned char authMode = theRequest.ReadByte();
	unsigned char encryptType = theRequest.ReadByte();
	unsigned short encryptFlags = theRequest.ReadShort();

	if(authMode!=1)
		return WS_PeerAuthServer_InvalidAuthMode;

	if(encryptType!=0 && encryptType!=1)
		return WS_PeerAuthServer_InvalidEncryptType;

	bool encrypted = encryptType==1;
	
	mAuthType = encrypted?AUTH_TYPE_PERSISTENT:AUTH_TYPE_PERSISTENT_NOCRYPT;

	unsigned short aLen = theRequest.ReadShort();
	if(mUseAuth2)
		mClientCertificate = new Auth2Certificate(theRequest.ReadBytes(aLen),aLen);
	else
		mClientCertificate = new AuthCertificate(theRequest.ReadBytes(aLen),aLen);
	
	if(!mClientCertificate->IsValid())
		return WS_PeerAuthServer_InvalidClientCertificate;
	else if(mClientCertificate->IsExpired(mPeerData->GetAuthDelta()))
		return WS_PeerAuthServer_ExpiredClientCertificate;
	else if(!mPeerData->Verify(mClientCertificate.get()))
		return WS_PeerAuthServer_FailedToVerifyClientCertificate;

	return GetChallenge1(theChallenge);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus PeerAuthServer::GetComplete(WONStatus theStatus, ByteBufferPtr &theComplete)
{
	mState = STATE_NOT_STARTED; // reset state now
	
	WriteBuffer aComplete(mLengthFieldSize);
	aComplete.AppendLong(203);					// Auth peer to peer service
	aComplete.AppendLong(53);					// Complete

	if(theStatus!=WS_Success)
	{
		aComplete.AppendShort(WS_CommServ_InvalidParameters); // failure status
		aComplete.AppendShort(1); // num errors
		aComplete.AppendString(WONStatusToString(theStatus));
	}
	else
	{
		aComplete.AppendShort(WS_Success);
		WriteBuffer anEncryptBuf;
		anEncryptBuf.AppendShort(mSecretA.GetKeyLen());
		anEncryptBuf.AppendBytes(mSecretA.GetKey(),mSecretA.GetKeyLen());

			
		ByteBufferPtr anEncrypt = mClientCertificate->GetPubKey().Encrypt(anEncryptBuf.data(),anEncryptBuf.length());
		if(anEncrypt.get()==NULL)
			return WS_PeerAuthServer_FailedToEncryptWithClientPubKey;

		aComplete.AppendShort(anEncrypt->length());
		aComplete.AppendBytes(anEncrypt->data(),anEncrypt->length());
		mSession = new AuthSession(mAuthType, 0, mSecretB, mLengthFieldSize);
	}
	
	theComplete = aComplete.ToByteBuffer();
	return WS_Success;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus PeerAuthServer::HandleChallenge2(ReadBuffer &theChallenge, ByteBufferPtr &theComplete)
{
	unsigned short anEncryptLen = theChallenge.ReadShort();
	ByteBufferPtr aDecrypt = mPeerData->GetPrivateKey().Decrypt(theChallenge.ReadBytes(anEncryptLen),anEncryptLen);
	if(aDecrypt.get()==NULL)
		return WS_PeerAuthServer_FailedToDecryptWithPrivateKey;

	ReadBuffer aBuf(aDecrypt->data(),aDecrypt->length());
	unsigned short aSecretBLen = aBuf.ReadShort();
	if(aSecretBLen!=mSecretB.GetKeyLen() || memcmp(mSecretB.GetKey(),aBuf.ReadBytes(aSecretBLen),aSecretBLen)!=0)
		return WS_PeerAuthServer_InvalidSecretB;

	if(!mSecretA.SetKey(aBuf.data()+aBuf.pos(),aBuf.Available()))
		return WS_PeerAuthServer_InvalidSecretA;

	return GetComplete(WS_Success, theComplete);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus PeerAuthServer::HandleRecvMsg(const void *inMsg, unsigned long inMsgLen, ByteBufferPtr &outMsg)
{
	outMsg = NULL;

	if(mState==STATE_NOT_STARTED)
		return WS_PeerAuthServer_NotStarted;

	WONStatus aStatus;
	try
	{
		ReadBuffer aMsg(inMsg,inMsgLen);
		DWORD aServiceType = aMsg.ReadLong();
		DWORD aMessageType = aMsg.ReadLong();

		if(aServiceType!=203) // Auth 1 peer to peer
			return WS_PeerAuthServer_InvalidServiceType;

		if(aMessageType==50)
		{
			if(mState!=STATE_AWAITING_REQUEST)
				return WS_PeerAuthServer_UnexpectedRequest;

			aStatus = HandleRequest(aMsg,outMsg);
			mState = STATE_AWAITING_CHALLENGE2;
		}
		else if(aMessageType==52)
		{
			if(mState!=STATE_AWAITING_CHALLENGE2)
				return WS_PeerAuthServer_UnexpectedChallenge;
			
			aStatus = HandleChallenge2(aMsg, outMsg);
		}
		else
			return WS_PeerAuthServer_InvalidMessage;
	}
	catch(ReadBufferException&)
	{
		aStatus = WS_PeerAuthServer_MsgUnpackFailure;
	}

	if(aStatus!=WS_Success)
		GetComplete(aStatus,outMsg);

	return aStatus;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

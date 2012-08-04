#include "PeerAuthClient.h"
#include "WONCommon/WriteBuffer.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
PeerAuthClient::PeerAuthClient()
{
	Reset();
	mUseAuth2 = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PeerAuthClient::Reset()
{
	mState = STATE_NOT_STARTED;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus PeerAuthClient::HandleChallenge1(ReadBuffer &theChallenge, ByteBufferPtr &challenge2)
{
	unsigned short aSecretLenWithLen = theChallenge.ReadShort();
	ByteBufferPtr aSecretBuf = mPeerData->GetPrivateKey().Decrypt(theChallenge.ReadBytes(aSecretLenWithLen),aSecretLenWithLen);
	if(aSecretBuf.get()==NULL)
		return WS_PeerAuthClient_Challenge1DecryptFailure;
		
	unsigned short aSecretLen = aSecretBuf->data()[0] | (aSecretBuf->data()[1]<<8);
	if(aSecretLen>aSecretBuf->length()-2)
		return WS_PeerAuthClient_Challenge1InvalidSecretLen;
		
	mSecretA.Create(8);		
	if(!mSecretB.SetKey(aSecretBuf->data()+2, aSecretLen))
		return WS_PeerAuthClient_Challenge1InvalidSecretKey;

	unsigned short aCertLen = theChallenge.ReadShort();

	if(mUseAuth2)
	{
		mServerCertificate = new Auth2Certificate(theChallenge.ReadBytes(aCertLen),aCertLen);
		if(!mServerCertificate->IsValid())
			return WS_PeerAuthClient_Challenge1CertificateUnpackFailure;
	}
	else
	{
		mServerCertificate = new AuthCertificate(theChallenge.ReadBytes(aCertLen),aCertLen);
		if(!mServerCertificate->IsValid())
			return WS_PeerAuthClient_Challenge1CertificateUnpackFailure;
	}

	if(!mPeerData->Verify(mServerCertificate.get()))
		return WS_PeerAuthClient_Challenge1CertificateVerifyFailure;

	return GetChallenge2(challenge2);
}
	

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus PeerAuthClient::GetChallenge2(ByteBufferPtr &challenge2)
{
	mState = STATE_AWAITING_COMPLETE;

	WriteBuffer aMsg(mLengthFieldSize);
		
	aMsg.AppendLong(203);								// Auth1 Peer To Peer
	aMsg.AppendLong(52);								// Auth1 Challenge 2

	WriteBuffer anEncryptBuf;
	anEncryptBuf.AppendShort(mSecretB.GetKeyLen());
	anEncryptBuf.AppendBytes(mSecretB.GetKey(), mSecretB.GetKeyLen());
	anEncryptBuf.AppendBytes(mSecretA.GetKey(), mSecretA.GetKeyLen());

	ByteBufferPtr anEncrypt = mServerCertificate->GetPubKey().Encrypt(anEncryptBuf.data(),anEncryptBuf.length());
	if(anEncrypt.get()==NULL)
		return WS_PeerAuthClient_Challenge2EncryptFailure;

	aMsg.AppendShort(anEncrypt->length());
	aMsg.AppendBytes(anEncrypt->data(),anEncrypt->length());
	challenge2 = aMsg.ToByteBuffer();
	return WS_Success;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus PeerAuthClient::HandleComplete(ReadBuffer &theComplete)
{
	short aStatus = theComplete.ReadShort();
	if(aStatus<0)
	{		
		unsigned short aNumErrors = theComplete.ReadShort();
		for(int i=0; i<aNumErrors; i++)
		{
			string anError;
			theComplete.ReadString(anError);
		}
		
		return (WONStatus)aStatus;
	}
		
	unsigned short aLen = theComplete.ReadShort();
	ByteBufferPtr aDecrypt = mPeerData->GetPrivateKey().Decrypt(theComplete.ReadBytes(aLen),aLen);
	if(aDecrypt.get()==NULL)
		return WS_PeerAuthClient_CompleteDecryptFailure;

	if(aDecrypt->length()<2)
		return WS_PeerAuthClient_CompleteInvalidSecretLen;
	
	aLen = (aDecrypt->data()[0] | (aDecrypt->data()[1]<<8));
	if(aLen>aDecrypt->length()-2 || aLen!=mSecretA.GetKeyLen() || memcmp(mSecretA.GetKey(),aDecrypt->data()+2,aLen)!=0)
		return WS_PeerAuthClient_CompleteInvalidSecretKey;	

	unsigned short aSessionId = 0;
	if(mAuthType==AUTH_TYPE_SESSION)
		aSessionId = theComplete.ReadShort();

	if(mAuthType!=AUTH_TYPE_PERSISTENT_NOCRYPT)
		mSession = new AuthSession(mAuthType, aSessionId, mSecretB, mLengthFieldSize);

	return WS_Success;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ByteBufferPtr PeerAuthClient::Start(const AuthPeerData *theData, AuthType theType, unsigned char theLengthFieldSize)
{
	mState = STATE_AWAITING_CHALLENGE;

	mPeerData = theData;
	mAuthType = theType;
	mLengthFieldSize = theLengthFieldSize;

	bool sessioned = (mAuthType==AUTH_TYPE_SESSION);
	bool encrypted = (sessioned || mAuthType==AUTH_TYPE_PERSISTENT);

	WriteBuffer aMsg(mLengthFieldSize);
	aMsg.AppendLong(203);								// Auth1 Peer To Peer
	aMsg.AppendLong(50);								// Auth1 Request


	aMsg.AppendByte(sessioned ? 2 : 1);					// 1 = persistent, 2 = sessioned	
	aMsg.AppendByte(encrypted ? 1 : 0);					// Encrypt Blowfish
	aMsg.AppendShort(sessioned? 0x0000 : 0x0001);		// 0x0000 None
														// 0x0001 Don't Use sequence numbers
														// 0x0002 Allow clear messages
														// 0x8000 Use client specified session key

	const AuthCertificateBase *aCert = mPeerData->GetCertificate2();
	if(aCert==NULL || !mUseAuth2)
	{
		mUseAuth2 = false;
		aCert = mPeerData->GetCertificate();
	}

	unsigned short aCertLen = aCert->GetRawLen();
	aMsg.AppendShort(aCertLen);
	aMsg.AppendBytes(aCert->GetRaw(),aCertLen);
	return aMsg.ToByteBuffer();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus PeerAuthClient::HandleRecvMsg(const void *inMsg, unsigned long inMsgLen, ByteBufferPtr &outMsg)
{
	outMsg = NULL;

	try
	{
		ReadBuffer aMsg(inMsg,inMsgLen);
		DWORD aServiceType = aMsg.ReadLong();
		DWORD aMessageType = aMsg.ReadLong();

		if(aServiceType!=203) // Auth 1 peer to peer
			return WS_PeerAuthClient_InvalidServiceType;

		if(aMessageType==53)
			return HandleComplete(aMsg);
		else if(aMessageType==51)
		{
			if(mState!=STATE_AWAITING_CHALLENGE)
				return WS_PeerAuthClient_UnexpectedChallenge;
			
			return HandleChallenge1(aMsg, outMsg);
		}
		else
			return WS_PeerAuthClient_InvalidMessage;
	}
	catch(ReadBufferException&)
	{
		return WS_PeerAuthClient_MsgUnpackFailure;
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

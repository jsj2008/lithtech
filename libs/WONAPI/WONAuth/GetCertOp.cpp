#include "GetCertOp.h"

#include "WONCommon/WriteBuffer.h"
#include "WONCrypt/MD5Digest.h"

using namespace std;
using namespace WONAPI;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GetCertOp::GetCertOp(AuthContext *theContext) : 
	ServerRequestOp(theContext->GetServerContext()), 
	mAuthContext(theContext)
{
	mCreateAccount = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetCertOp::RunHook()
{
	if(!mAuthContext->GetVerifierKey().IsPublic())
	{
		Finish(WS_GetCert_NeedVerifierKey);
		return;
	}

	AuthPeerDataPtr aPeerData = mAuthContext->GetPeerData();
	mKeyBlock = aPeerData->GetPubKeyBlock();
	mAuthDelta = aPeerData->GetAuthDelta();
	mAuthContext->LoadSecretList();
	ServerRequestOp::RunHook();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetCertOp::CleanupHook()
{
	// Give the pub key block to the AuthContext if we managed to at least get that
	if(!Succeeded() && mKeyBlock->IsValid() && !mAuthContext->GetPeerData()->GetPubKeyBlock()->IsValid())
		mAuthContext->SetPeerData(new AuthPeerData(mKeyBlock));

	ServerRequestOp::CleanupHook();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetCertOp::Reset()
{
	mState = GETTING_PUB_KEYS;
	mChallengeSeed.erase();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetCertOp::SendPubKeyRequest()
{
	mState = GETTING_PUB_KEYS;

	WriteBuffer aMsg(4);
	aMsg.AppendByte(5);			// Small Message
	aMsg.AppendShort(12);		// Auth Server Service
	aMsg.AppendShort(1);		// Get Pub Keys
	mRequest = aMsg.ToByteBuffer();
	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetCertOp::SendLoginRequest()
{
	WriteBuffer aMsg(4);
	aMsg.AppendByte(5);										// Small message
	aMsg.AppendShort(12);									// Auth Login Service
	aMsg.AppendShort(3);									// Auth 2 Login Request 				
	aMsg.AppendShort(mKeyBlock->GetId());					// Pub Key Block Id
	
	const ElGamal &aPubKey = mKeyBlock->GetCurrentKey();
	mSessionKey.Create(8);

	ByteBufferPtr anEncryptedSessionKey = aPubKey.Encrypt(mSessionKey.GetKey(),mSessionKey.GetKeyLen());
	if(anEncryptedSessionKey.get()==NULL)	
		return WS_GetCert_PubKeyEncryptFailure;

	aMsg.AppendShort(anEncryptedSessionKey->length());
	aMsg.AppendBytes(anEncryptedSessionKey->data(), anEncryptedSessionKey->length());

	WriteBuffer aLoginData(0);
	aLoginData.AppendBytes(mSessionKey.GetKey(),2);			// random pad
	aLoginData.AppendShort(mKeyBlock->GetId());

	int flags = 0x0002 | 0x0004 | 0x0008;
	if(mCreateAccount)
		flags |= 0x0001;

	aLoginData.AppendLong(flags);								// flags --> 1 = Create User, 2 = Req Private Key, 4 = Req Auth 1 Cert, 8 = Req Auth 2 Cert
	aLoginData.AppendWString(mAuthContext->GetUserName());		// username
	aLoginData.AppendWString(mAuthContext->GetPassword());		// password
	aLoginData.AppendWString(mNewPassword);						// new password
	mAuthContext->AppendCommunityData(aLoginData);
	
	ByteBufferPtr anEncryptedLoginData = mSessionKey.Encrypt(aLoginData.data(),aLoginData.length());

	aMsg.AppendShort(anEncryptedLoginData->length());
	aMsg.AppendBytes(anEncryptedLoginData->data(), anEncryptedLoginData->length());
	mRequest = aMsg.ToByteBuffer();

	mState = GETTING_CHALLENGE;
	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetCertOp::SendLoginConfirm()
{
	WriteBuffer aMsg(4);
	aMsg.AppendByte(5);										// Small message
	aMsg.AppendShort(12);									// Auth Login Service
	aMsg.AppendShort(5);									// Auth 2 Login Confirm 				

	WriteBuffer aDataBlock;
	mAuthContext->AppendHashes(aDataBlock,mChallengeSeed);

	ByteBufferPtr anEncrypt = mSessionKey.Encrypt(aDataBlock.data(),aDataBlock.length());
	aMsg.AppendShort(anEncrypt->length());
	aMsg.AppendBytes(anEncrypt->data(),anEncrypt->length());
	mRequest = aMsg.ToByteBuffer();

	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetCertOp::GetNextRequest()
{	
 	if(!mKeyBlock->IsValid()) 
		return SendPubKeyRequest();
	else if(mState==GETTING_CHALLENGE)
		return SendLoginConfirm();
	else
		return SendLoginRequest();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetCertOp::HandleLoginReply(ReadBuffer &theMsg)
{

	AuthCertificatePtr aCert1;
	Auth2CertificatePtr aCert2;
	ElGamal aPrivKey;
	time_t anAuthDelta = 0;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	if(aStatus<0) // Failure status
	{
		if(!mChallengeSeed.empty())
			mAuthContext->RemoveSecret(mChallengeSeed.data());

		if(aStatus==-1) // unknown auth error
		{
			SetLastServerError(aStatus);
			return WS_ServerReq_TryNextServer;
		}

		return aStatus;
	}
	
	unsigned char anErrorCount = theMsg.ReadByte();
	unsigned long i;
	for(i=0; i<anErrorCount; i++)
	{
		string anErrorString;
		theMsg.ReadString(anErrorString);
	}
	
	unsigned char aNumClearEntries = theMsg.ReadByte();

	for(i=0; i<aNumClearEntries; i++)
	{
		unsigned char aType = theMsg.ReadByte();
		unsigned short aLen = theMsg.ReadShort();
		const void *data = theMsg.ReadBytes(aLen);
		switch(aType)
		{
			case 1:	
				aCert1 = new AuthCertificate(data,aLen);
				if(!aCert1->IsValid() || !mKeyBlock->Verify(aCert1)) 
				{
					SetLastServerError(WS_GetCert_InvalidCertificate);
					return WS_ServerReq_TryNextServer; 
				}
				else
					anAuthDelta = aCert1->ComputeAuthDelta();
				break;
			
			case 3: 
			{
				AuthPubKeyBlockPtr aKeyBlock = new AuthPubKeyBlock(data,aLen);
				if(!aKeyBlock->IsValidAndVerified(mAuthContext->GetVerifierKey()))
				{
					SetLastServerError(WS_GetCert_InvalidPubKeyBlock);
					return WS_ServerReq_TryNextServer;
				}
				else
					mKeyBlock = aKeyBlock;	
			}
			break;

			case 6:
				aCert2 = new Auth2Certificate(data,aLen);
				if(!aCert2->IsValid() || !mKeyBlock->Verify(aCert2))
				{
					SetLastServerError(WS_GetCert_InvalidCertificate);
					return WS_ServerReq_TryNextServer; 
				}
				else
					anAuthDelta = aCert2->ComputeAuthDelta();
				break;
		}
	}
	
	// Commit secrets and clean up secret file using auth server clock
	time_t anAuthTime = 0, aCertDuration;
	if(aCert2.get()!=NULL)
	{
		anAuthTime = aCert1->GetIssueTime();
		aCertDuration = aCert1->GetExpireTime() - aCert1->GetIssueTime();
	}
	else if(aCert1.get()!=NULL)
	{
		anAuthTime = aCert2->GetIssueTime();
		aCertDuration = aCert2->GetExpireTime() - aCert2->GetIssueTime();
	}

	mAuthContext->CommitSecrets(anAuthTime, aCertDuration);

	
	int aNumLeft = theMsg.length() - theMsg.pos();

	ByteBufferPtr aDecryptBuf = mSessionKey.Decrypt(theMsg.ReadBytes(aNumLeft),aNumLeft);
	if(aDecryptBuf.get()==NULL)
	{
		SetLastServerError(WS_GetCert_DecryptFailure);
		return WS_ServerReq_TryNextServer;
	}
	
	ReadBuffer aDecrypt(aDecryptBuf->data(),aDecryptBuf->length());		
	unsigned char aNumCryptEntries = aDecrypt.ReadByte();
	
	for(i=0; i<aNumCryptEntries; i++)
	{
		unsigned char aType = aDecrypt.ReadByte();
		unsigned short aLen = aDecrypt.ReadShort();
		const char *data = (const char*)aDecrypt.ReadBytes(aLen);
		switch(aType)
		{
			case 2: 
				if(!aPrivKey.SetPrivateKey(data,aLen))
				{
					SetLastServerError(WS_GetCert_InvalidPrivateKey);
					return WS_ServerReq_TryNextServer;
				}
				break;

			case 4:
				if((aLen!=mSessionKey.GetKeyLen()+2) || memcmp(data+2,mSessionKey.GetKey(),aLen-2)!=0)
				{
					SetLastServerError(WS_GetCert_InvalidSecretConfirm);
					return WS_ServerReq_TryNextServer;
				}
				break;

			case 5: 
				//ReadNickNameInfo(aDecryptIn); 
				break;
		}
	}

	if(aCert1.get()==NULL || aCert2.get()==NULL)
	{
		SetLastServerError(WS_GetCert_MissingCertificate);
		return WS_ServerReq_TryNextServer;
	}

	mPeerData = new AuthPeerData(aCert1, mKeyBlock, aPrivKey, anAuthDelta, aCert2);
	if(!mNewPassword.empty())
		mAuthContext->SetPassword(mNewPassword);

	mAuthContext->SetPeerData(mPeerData);	
	mAuthContext->ClearOneTimeData();
	return WS_Success;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetCertOp::HandleLoginChallenge(ReadBuffer &theMsg)
{
	unsigned short aLen = theMsg.ReadShort();
	ByteBufferPtr aDecrypt = mSessionKey.Decrypt(theMsg.ReadBytes(aLen),aLen);
	if(aDecrypt.get()==NULL)
	{
		SetLastServerError(WS_GetCert_DecryptFailure);
		return WS_ServerReq_TryNextServer;
	}

	ReadBuffer aBuf(aDecrypt->data(),aDecrypt->length());
	mChallengeSeed.assign((const unsigned char*)aBuf.ReadBytes(16),16);
	mAuthContext->AddSecret(mChallengeSeed.data());
	return WS_ServerReq_Send;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetCertOp::CheckResponse()
{
	ReadBuffer aMsg(mResponse->data(),mResponse->length());
	unsigned char aHeaderType = aMsg.ReadByte();
	unsigned short aServiceType = aMsg.ReadShort();
	unsigned short aMessageType = aMsg.ReadShort();

	if(aHeaderType!=5 || aServiceType!=12)
		return InvalidReplyHeader();

	if(mState==GETTING_PUB_KEYS)
	{
		if(aMessageType!=2) // GetPubKeysReply
			return InvalidReplyHeader();

		WONStatus aStatus = (WONStatus)aMsg.ReadShort();
		if(aStatus<0) // shouldn't get this
		{
			SetLastServerError(aStatus);
			return WS_ServerReq_TryNextServer;
		}

		unsigned short aLen = aMsg.ReadShort();
		AuthPubKeyBlockPtr aKeyBlock = new AuthPubKeyBlock(aMsg.ReadBytes(aLen),aLen);
		if(!aKeyBlock->IsValidAndVerified(mAuthContext->GetVerifierKey()))
		{
			SetLastServerError(WS_GetCert_InvalidPubKeyBlock);
			return WS_ServerReq_TryNextServer;
		}
		else
		{
			mKeyBlock = aKeyBlock;
			return WS_ServerReq_Send;
		}
	}

	
	
	if(aMessageType==6)
		return HandleLoginReply(aMsg);
	
	if(aMessageType!=4)
		return InvalidReplyHeader();

	if(mState!=GETTING_CHALLENGE)
	{
		SetLastServerError(WS_GetCert_UnexpectedLoginChallenge);
		return WS_ServerReq_TryNextServer;
	}

	return HandleLoginChallenge(aMsg);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

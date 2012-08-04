#include "CreateAccountOp.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CreateAccountOp::Init()
{
	mLengthFieldSize = 4;
	mProfileData = new ProfileData;
	mSymKey.Create(8);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CreateAccountOp::CreateAccountOp(ServerContext *theProfileContext) : ServerRequestOp(theProfileContext)
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CreateAccountOp::CreateAccountOp(const IPAddr &theAddr) : ServerRequestOp(theAddr)
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CreateAccountOp::Reset()
{
	mState = State_GettingPubKey;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus CreateAccountOp::GetNextRequest()
{
	WriteBuffer aBuf(mLengthFieldSize);

	if(mState==State_GettingPubKey)
	{
		aBuf.AppendByte(5);			// small message
		aBuf.AppendShort(14);		// DBProxy Service
		aBuf.AppendShort(2);		// Get Pub Key
		aBuf.AppendShort(1);		// Sub Message Type (doesn't matter, just must be non-zero)
		mRequest = aBuf.ToByteBuffer();
		return WS_ServerReq_Recv;
	}
	else if(mState==State_SettingSessionKey)
	{
		aBuf.AppendByte(5);			// small message
		aBuf.AppendShort(14);		// DBProxy Service
		aBuf.AppendShort(4);		// Set Session Key
		aBuf.AppendShort(1);		// Sub Message Type (doesn't matter, just must be non-zero)
		ByteBufferPtr anEncryptedSymKey = mPubKey.Encrypt(mSymKey.GetKey(), mSymKey.GetKeyLen());
		aBuf.AppendBuffer(anEncryptedSymKey,2);
		mRequest = aBuf.ToByteBuffer();

		mState = State_CreatingAccount;
		return WS_ServerReq_Send;
	}
	else
	{
		aBuf.AppendByte(5);			// small message
		aBuf.AppendShort(14);		// DBProxy Service
		aBuf.AppendShort(5);		// DBProxy Encrypted/Unauth Message
		aBuf.AppendShort(1);		// Sub Message Type (doesn't matter, just must be non-zero)

		WriteBuffer anEncrypt;
		anEncrypt.AppendByte(5);	// small message
		anEncrypt.AppendShort(14);	// DBProxy Service
		anEncrypt.AppendShort(1002);// DBProxy Create WON User
		anEncrypt.AppendShort(2);	// Create Account

		// Encrypt UserName and Password with SymKey
		anEncrypt.AppendWString(mUserName);
		anEncrypt.AppendWString(mPassword);
		anEncrypt.AppendString(mEmail);
		mProfileData->PackRequestData(anEncrypt, true);

		ByteBufferPtr anEncryptBuf = mSymKey.Encrypt(anEncrypt.data(), anEncrypt.length());
		aBuf.AppendBuffer(anEncryptBuf, 0);
		mRequest = aBuf.ToByteBuffer();

		return WS_ServerReq_Recv;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus CreateAccountOp::CheckResponse()
{
	ReadBuffer aMsg(mResponse->data(),mResponse->length());
	unsigned char aHeaderType = aMsg.ReadByte();
	unsigned short aServiceType = aMsg.ReadShort();
	unsigned short aMessageType = aMsg.ReadShort();
	unsigned short aSubMessageType = aMsg.ReadShort();
	WONStatus aStatus = (WONStatus)aMsg.ReadShort();
	aMsg.ReadString(mErrorString);
	unsigned char aSeq = aMsg.ReadByte();

	if(aHeaderType!=5 || aServiceType!=14)
		return InvalidReplyHeader();

	if(aStatus!=WS_Success)
		return aStatus;

	if(mState==State_GettingPubKey)
	{
		if(aMessageType!=3)
			return InvalidReplyHeader();

		unsigned short aKeyLen = aMsg.ReadShort();
		if(!mPubKey.SetPublicKey(aMsg.ReadBytes(aKeyLen),aKeyLen))
		{
			SetLastServerError(WS_InvalidPublicKey);
			return WS_ServerReq_TryNextServer;
		}
		else
		{
			mState = State_SettingSessionKey;
			return WS_ServerReq_Send;
		}
	}
	else 
	{
		if(aMessageType!=1002 || aSubMessageType!=3) // Create Account Reply
			return InvalidReplyHeader();

		//mProfileData->UnpackSetReply(aMsg); no optional info yet
		return (WONStatus)aStatus;
	}
}

#include <time.h>

#include "AuthSession.h"
#include "WONCommon/WriteBuffer.h"
#include "WONCommon/LittleEndian.h"

using namespace WONAPI;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AuthSession::AuthSession(AuthType theType, unsigned short theId, Blowfish theKey, 
						 unsigned char theLengthFieldSize) : mAuthType(theType), mId(theId), 
						 mKey(theKey), mLengthFieldSize(theLengthFieldSize), mInSeq(0), mOutSeq(0) 
{
	mLastUseTime = time(NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AuthSession::~AuthSession()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus AuthSession::Encrypt(ByteBufferPtr &theMsg)
{
	mLastUseTime = time(NULL);

	if(mAuthType==AUTH_TYPE_NONE || mAuthType==AUTH_TYPE_PERSISTENT_NOCRYPT)
		return WS_Success;

	WriteBuffer aMsg(mLengthFieldSize);
	aMsg.AppendByte(12); // encrypted message

	WriteBuffer aSeqBuf;
	const char *aBuf = theMsg->data() + mLengthFieldSize;
	unsigned short aLen = theMsg->length() - mLengthFieldSize;

	if(mAuthType==AUTH_TYPE_SESSION)
	{
		aMsg.AppendShort(mId);
		aSeqBuf.AppendShort(++mOutSeq);
		aSeqBuf.AppendBytes(aBuf,aLen);
		aBuf = aSeqBuf.data();
		aLen = aSeqBuf.length();
	}

	ByteBufferPtr anEncrypt = mKey.Encrypt(aBuf,aLen);
	if(anEncrypt.get()==NULL)
		return WS_AuthSession_EncryptFailure;
	
	aMsg.AppendBytes(anEncrypt->data(),anEncrypt->length());
	theMsg = aMsg.ToByteBuffer();
	return WS_Success;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus AuthSession::Decrypt(ByteBufferPtr &theMsg)
{
	mLastUseTime = time(NULL);

	try
	{
		if(mAuthType==AUTH_TYPE_NONE || mAuthType==AUTH_TYPE_PERSISTENT_NOCRYPT)
			return WS_Success;

		ReadBuffer anIn(theMsg->data(),theMsg->length());
		WriteBuffer anOut;
		unsigned char headerType = anIn.ReadByte();
		switch (headerType)
		{
			case 2:							break;	//WONMsg::EncryptedService:
			case 4:	anOut.AppendByte(3);	break;	//WONMsg::MiniEncryptedService:
			case 6:	anOut.AppendByte(5);	break;	//WONMsg::SmallEncryptedService:
			case 8:	anOut.AppendByte(7);	break;	//WONMsg::LargeEncryptedService:
			case 12:						break;	//WONMsg::HeaderEncryptedService:

			default:
				return WS_Success;
		}

		bool sessioned = mAuthType==AUTH_TYPE_SESSION;

		if(sessioned)
		{
			unsigned short aSessionId = anIn.ReadShort();
			if(aSessionId!=mId)
				return WS_AuthSession_DecryptSessionIdMismatch;
		}

		ByteBufferPtr aDecrypt = mKey.Decrypt(anIn.data() + anIn.pos(), anIn.length() - anIn.pos());
		if(aDecrypt.get()==NULL)
			return WS_AuthSession_DecryptFailure;

		if(sessioned)
		{
			if(aDecrypt->length()<2)
				return WS_AuthSession_DecryptBadLen;

			if(++mInSeq!=ShortFromLittleEndian(*(unsigned short*)aDecrypt->data())) // sequence mismatch
				return WS_AuthSession_DecryptInvalidSequenceNum;

			anOut.AppendBytes(aDecrypt->data()+2,aDecrypt->length()-2);
		}
		else
			anOut.AppendBytes(aDecrypt->data(),aDecrypt->length());

		theMsg = anOut.ToByteBuffer();
		return WS_Success;
	}
	catch(ReadBufferException&)
	{
		return WS_AuthSession_DecryptUnpackFailure;
	}
}

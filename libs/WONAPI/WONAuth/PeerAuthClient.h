#ifndef __WON_PEERAUTHCLIENT_H__
#define __WON_PEERAUTHCLIENT_H__
#include "WONShared.h"

#include "WONCrypt/Blowfish.h"
#include "AuthPeerData.h"
#include "AuthSession.h"

namespace WONAPI
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class PeerAuthClient
{
private:
	AuthPeerDataPtr mPeerData;
	AuthType mAuthType;
	Blowfish mSecretA;
	Blowfish mSecretB;
	unsigned char mLengthFieldSize;

	bool mUseAuth2;
	AuthCertificateBasePtr mServerCertificate;
	AuthSessionPtr mSession;

public:
	enum PeerAuthClientState
	{
		STATE_NOT_STARTED = 0,
		STATE_AWAITING_CHALLENGE = 1,
		STATE_AWAITING_COMPLETE = 2
	} mState;

private:
	WONStatus HandleChallenge1(ReadBuffer &theChallenge, ByteBufferPtr &challenge2);
	WONStatus GetChallenge2(ByteBufferPtr &challenge2);
	WONStatus HandleComplete(ReadBuffer &theComplete);

public:
	PeerAuthClient();
	void Reset();
	ByteBufferPtr Start(const AuthPeerData *theData, AuthType theType, unsigned char theLengthFieldSize);	
	WONStatus HandleRecvMsg(const void *inMsg, unsigned long inMsgLen, ByteBufferPtr &outMsg);

	const AuthCertificateBase* GetServerCertificate() { return mServerCertificate; }
	AuthSession* GetSession() { return mSession; }

	PeerAuthClientState GetState() { return mState; }

	void SetUseAuth2(bool useAuth2) { mUseAuth2 = useAuth2; }
};

}; // namespace WONAPI

#endif

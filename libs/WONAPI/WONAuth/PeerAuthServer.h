#ifndef __WON_PEERAUTHSERVER_H__
#define __WON_PEERAUTHSERVER_H__
#include "WONShared.h"

#include "WONCrypt/Blowfish.h"
#include "AuthPeerData.h"
#include "AuthSession.h"

namespace WONAPI
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class PeerAuthServer
{
private:
	AuthPeerDataPtr mPeerData;
	AuthType mAuthType;
	Blowfish mSecretA;
	Blowfish mSecretB;
	unsigned char mLengthFieldSize;

	AuthCertificateBasePtr mClientCertificate;
	AuthSessionPtr mSession;
	bool mUseAuth2;

public:
	enum PeerAuthServerState
	{
		STATE_NOT_STARTED = 0,
		STATE_AWAITING_REQUEST = 1,
		STATE_AWAITING_CHALLENGE2 = 2
	} mState;

private:
	WONStatus HandleRequest(ReadBuffer &theRequest, ByteBufferPtr &theChallenge);
	WONStatus GetChallenge1(ByteBufferPtr &theChallenge);
	WONStatus HandleChallenge2(ReadBuffer &theChallenge, ByteBufferPtr &theComplete);
	WONStatus GetComplete(WONStatus theStatus, ByteBufferPtr &theComplete);

public:
	PeerAuthServer();

	void Start(const AuthPeerData *theData, unsigned char theLengthFieldSize);	
	WONStatus HandleRecvMsg(const void *inMsg, unsigned long inMsgLen, ByteBufferPtr &outMsg);

	const AuthCertificateBase* GetClientCertificate() { return mClientCertificate; }
	AuthSession* GetSession() { return mSession; }
	PeerAuthServerState GetState() { return mState; }

	void SetUseAuth2(bool useAuth2) { mUseAuth2 = useAuth2; }
};

}; // namespace WONAPI

#endif

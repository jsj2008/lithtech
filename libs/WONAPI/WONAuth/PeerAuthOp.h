#ifndef __WON_PEERAUTHOP_H__
#define __WON_PEERAUTHOP_H__
#include "WONShared.h"

#include "WONCrypt/Blowfish.h"
#include "WONServer/ServerOp.h"
#include "PeerAuthClient.h"

namespace WONAPI
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class PeerAuthOp : public ServerOp
{
private:
	IPAddr mConnectAddr;
	WONStatus mGetCertStatus;
	AuthPeerDataPtr mPeerData;
	PeerAuthClient mPeerAuthClient;

	enum PeerAuthTrack
	{
		PeerAuth_Track_Socket = 1,
		PeerAuth_Track_GetCert = 2,
	};
	virtual bool CallbackHook(AsyncOp *theOp, int theParam);

	bool CheckStatus(WONStatus theStatus);
	void AsyncSend(const ByteBuffer *theMsg);
	void AsyncConnect();
	void AsyncRefreshCert();
	void AsyncRetry();
	void AsyncRun();

	void Success();

protected:
	virtual ~PeerAuthOp();
	virtual void RunHook();
	virtual void CleanupHook();

public:
	PeerAuthOp(const IPAddr &theAddr, AuthContext *theAuthContext, AuthType theType, BlockingSocket *theSocket, bool useAuth2 = true);
	AuthSession* GetSession() { return mPeerAuthClient.GetSession(); }

	BlockingSocket* GetSocket() { return mSocket; }
	Auth2Certificate* GetServerCertificate() { return (Auth2Certificate*)mPeerAuthClient.GetServerCertificate(); }

	WONStatus GetGetCertStatus() const { return mGetCertStatus; }
};

typedef SmartPtr<PeerAuthOp> PeerAuthOpPtr;

}; // namespace WONAPI

#endif

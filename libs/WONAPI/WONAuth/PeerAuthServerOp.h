#ifndef __WON_PeerAuthServerOp_H__
#define __WON_PeerAuthServerOp_H__
#include "WONShared.h"

#include "WONCrypt/Blowfish.h"
#include "WONServer/ServerOp.h"
#include "PeerAuthServer.h"

namespace WONAPI
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class PeerAuthServerOp : public ServerOp
{
private:
	IPAddr mConnectAddr;
	PeerAuthServer mPeerAuthServer;

	enum PeerAuthTrack
	{
		PeerAuth_Track_Socket = 1,
		PeerAuth_Track_GetCert = 2,
	};
	virtual bool CallbackHook(AsyncOp *theOp, int theParam);

	bool CheckStatus(WONStatus theStatus);
	void AsyncRun();

	void Success();

protected:
	virtual ~PeerAuthServerOp();
	virtual void RunHook();
	virtual void CleanupHook();

public:
	PeerAuthServerOp(AuthContext *theAuthContext, BlockingSocket *theSocket);
	AuthSession* GetSession() { return mPeerAuthServer.GetSession(); }

	BlockingSocket* GetSocket() { return mSocket; }
	Auth2Certificate* GetClientCertificate() { return (Auth2Certificate*)mPeerAuthServer.GetClientCertificate(); }
};

typedef SmartPtr<PeerAuthServerOp> PeerAuthServerOpPtr;

}; // namespace WONAPI

#endif

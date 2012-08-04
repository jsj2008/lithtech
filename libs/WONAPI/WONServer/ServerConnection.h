#ifndef __WON_SERVERCONNECTION_H__
#define __WON_SERVERCONNECTION_H__
#include "WONShared.h"

#include "WONAuth/PeerAuthOp.h"
#include "WONCommon/SmartPtr.h"
#include "WONSocket/BlockingSocket.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ServerConnection : public BlockingSocket
{
private:
	AuthContextPtr mAuthContext;
	AuthType mAuthType;
	PeerAuthOpPtr mPeerAuthOp;

	OpCompletionBasePtr mConnectCompletion;

	AsyncOpPtr mKeepAliveOp;
	DWORD mKeepAlivePeriod; // in seconds
	void StartKeepAlive();

protected:
	static void StaticConnectCallback(AsyncOpPtr theOp, RefCountPtr theParam);
	virtual void ConnectCallback(PeerAuthOp *theOp);

	static void StaticMsgCallback(AsyncOpPtr theOp, RefCountPtr theParam);
	virtual void MsgCallback(const ByteBuffer* theMsg);

	static void StaticKeepAliveCallback(AsyncOpPtr theOp, RefCountPtr theParam);
	void KeepAliveCallback(AsyncOp *theOp);
	void SendNoOp(); // used for keep alives

	virtual void CloseHook();
	virtual void KillHook();

public:
	explicit ServerConnection();
	void SetAuth(AuthContext *theAuthContext, AuthType theAuthType = AUTH_TYPE_PERSISTENT);
	
	void SetConnectCompletion(OpCompletionBase *theCompletion);
	void ConnectAsync(const IPAddr &theAddr, DWORD theTimeout = OP_TIMEOUT_INFINITE);
	WONStatus Connect(const IPAddr &theAddr, DWORD theTimeout = OP_TIMEOUT_INFINITE) { return ConnectBlock(theAddr,theTimeout); } // to prevent confusion with BlockingSocket::Connect and AsyncSocket::Connect
	WONStatus ConnectBlock(const IPAddr &theAddr, DWORD theTimeout = OP_TIMEOUT_INFINITE);

	void SetKeepAlivePeriod(DWORD theSeconds); // 0 = don't send keep alive

};

typedef SmartPtr<ServerConnection> ServerConnectionPtr;

}; // namespace WONAPI

#endif

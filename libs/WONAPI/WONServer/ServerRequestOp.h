#ifndef __WON_SERVERREQUESTOP_H__
#define __WON_SERVERREQUESTOP_H__
#include "WONShared.h"


#include "WONCommon/ByteBuffer.h"
#include "WONSocket/BlockingSocket.h"
#include "ServerOp.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class ServerRequestOp : public ServerOp
{
private:
	AddrList mAddrList;
	AddrList::iterator mAddrItr;
	AuthSessionPtr mCurSession;
	bool mReusingSession;
	bool mSessionExpired;
	IPAddr mCurAddr;
	bool mUseAuth2;

	WONStatus mGetCertStatus;

	bool mUseLastServerErrorForFinishStatus;
	WONStatus mLastServerError;

	enum ServerReqTrack
	{
		ServerReq_Track_Socket = 1,
		ServerReq_Track_PeerAuth = 2,
	};

	void QueueSocketOp(SocketOp *theOp, DWORD theTimeout = OP_TIMEOUT_INFINITE);
	void Init();	

protected:
	ServerContextPtr mServerContext;
	ByteBufferPtr mRequest;
	ByteBufferPtr mResponse;

	const IPAddr& GetCurAddr() const { return mCurAddr; }
protected:

	WONStatus Send();
	WONStatus Recv();
	bool TryConnect();
	bool TryPeerToPeerAuth();
	bool CheckExpiredSession(); 
	bool CallbackHook(AsyncOp *theOp, int theParam);
	bool TryNextServer();
	void FinishSaveSession(WONStatus theStatus);
	void SetLastServerError(WONStatus theError);
	WONStatus InvalidReplyHeader();

protected:
	virtual WONStatus GetNextRequest() { return WS_ServerReq_Recv; }
	virtual WONStatus CheckResponse() { return WS_Success; }
	virtual void RunHook();
	virtual void CleanupHook();
	virtual void Reset() { }
	virtual ~ServerRequestOp() { }
	
public:
	ServerRequestOp(ServerContext *theContext);
	ServerRequestOp(const IPAddr &theAddr);

	void SetServerContext(ServerContext *theContext);
	void SetAddr(const IPAddr &theAddr);
	IPAddr GetAddr() const;

	void SetRequest(const ByteBuffer *theRequest) { mRequest = theRequest; }

	const ByteBuffer* GetRequest() const { return mRequest; }
	const ByteBuffer* GetResponse() const { return mResponse; }

	void SetUseAuth2(bool useAuth2) { mUseAuth2 = useAuth2; }
	WONStatus GetGetCertStatus() const { return mGetCertStatus; }
	WONStatus GetLastServerError() const { return mLastServerError; }
};

typedef SmartPtr<ServerRequestOp> ServerRequestOpPtr;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif

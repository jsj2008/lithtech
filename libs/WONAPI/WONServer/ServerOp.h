#ifndef __WON_SERVEROP_H__
#define __WON_SERVEROP_H__
#include "WONShared.h"

#include "WONCommon/AsyncOpTracker.h"
#include "WONSocket/BlockingSocket.h"
#include "WONAuth/AuthContext.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class GetCertOp;
class PeerAuthOp;

class ServerOp : public AsyncOpWithTracker
{
protected:
	BlockingSocketPtr mSocket;

	AuthContextPtr mAuthContext;
	AuthType mAuthType;
	unsigned char mLengthFieldSize;


	DWORD mMaxConnectTime, mMaxSendTime, mMaxRecvTime;

	DWORD ConnectTime();
	DWORD SendTime();
	DWORD RecvTime();

	void SendMsgAsync(ByteBufferPtr theMsg, int theTrackId = 0);
	void RecvMsgAsync(int theTrackId = 0);

public:
	ServerOp();
	void SetMaxTimes(int theConnectTime, int theSendTime, int theRecvTime);

	void SetMaxConnectTime(int theConnectTime) { mMaxConnectTime = theConnectTime; }
	void SetMaxSendTime(int theSendTime) { mMaxSendTime = theSendTime; }
	void SetMaxRecvTime(int theRecvTime) { mMaxRecvTime = theRecvTime; }

	DWORD GetMaxConnectTime() { return mMaxConnectTime; }
	DWORD GetMaxSendTime() { return mMaxSendTime; }
	DWORD GetMaxRecvTime() { return mMaxRecvTime; }

	void SetLengthFieldSize(unsigned char theLengthFieldSize) { mLengthFieldSize = theLengthFieldSize; }
	unsigned char GetLengthFieldSize() { return mLengthFieldSize; }

	void SetAuthType(AuthType theType, AuthContext *theContext = NULL) { mAuthType = theType; mAuthContext = theContext; }

	void CopyMaxTimes(ServerOp *theCopyFrom);
};
typedef SmartPtr<ServerOp> ServerOpPtr;


}; // namespace WONAPI

#endif

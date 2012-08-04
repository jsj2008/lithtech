#ifndef __WON_RECVBYTESFROMOP_H__
#define __WON_RECVBYTESFROMOP_H__
#include "WONShared.h"
#include "SocketOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RecvBytesFromOp : public SocketOp
{
protected:
	char *mCurRecv;
	ByteBufferPtr mRecvBytes;
	unsigned short mMaxBytes;
	IPAddr mAddr;

protected:
	virtual void RunHook();
	virtual WONStatus Continue();
	virtual SocketOp* Duplicate() { return new RecvBytesFromOp(mMaxBytes,mSocket); }

public:
	RecvBytesFromOp(unsigned short theMaxBytes, AsyncSocket *theSocket = NULL);
	virtual ~RecvBytesFromOp();

	ByteBufferPtr GetBytes() const { return mRecvBytes; }
	const IPAddr& GetAddr() const { return mAddr; }
};

typedef SmartPtr<RecvBytesFromOp> RecvBytesFromOpPtr;

} // namespace WONAPI

#endif

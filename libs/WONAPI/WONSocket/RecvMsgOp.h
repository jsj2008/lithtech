#ifndef __WON_RECVMSGOP_H__
#define __WON_RECVMSGOP_H__
#include "WONShared.h"
#include "RecvBytesOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RecvMsgOp : public RecvBytesOp
{
protected:
	virtual WONStatus Start();
	virtual WONStatus Continue();
	WONStatus StartMsgRecv();
	WONStatus ExtractMsg();
	ByteBufferPtr mMsg;

	enum
	{
		RECEIVING_LENGTH = 1,
		RECEIVING_MESSAGE = 2
	} mState;

	virtual SocketOp* Duplicate() { return new RecvMsgOp(mSocket); }

public:	
	RecvMsgOp(AsyncSocket *theSocket = NULL) : RecvBytesOp(0, theSocket) {}
	ByteBufferPtr GetMsg() { return mMsg; }
};

typedef SmartPtr<RecvMsgOp> RecvMsgOpPtr;


}; // namespace WONAPI

#endif

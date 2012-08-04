#ifndef __WON_RECVCRMSGOP_H__
#define __WON_RECVCRMSGOP_H__
#include "WONShared.h"
#include "SocketOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RecvCRMsgOp : public SocketOp
{
protected:
	WriteBuffer mCurBytes;
	ByteBufferPtr mMsg;

	virtual SocketOp* Duplicate() { return new RecvCRMsgOp(mSocket); }
	virtual WONStatus Start();
	virtual WONStatus Continue();

public:
	RecvCRMsgOp(AsyncSocket *theSocket = NULL) : SocketOp(theSocket) { mSocketEvent[SocketEvent_Read] = true; }
	ByteBufferPtr GetMsg() { return mMsg; }
};

typedef SmartPtr<RecvCRMsgOp> RecvCRMsgOpPtr;

}; // namespace WONAPI

#endif

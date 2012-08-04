#ifndef __WON_SENDBYTESOP_H__
#define __WON_SENDBYTESOP_H__
#include "WONShared.h"
#include "SocketOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SendBytesOp : public SocketOp
{
protected:
	ReadBuffer mBytes;
	ByteBufferPtr mBytesBuf;

protected:
	virtual WONStatus Start();
	virtual WONStatus Continue();

public:
	SendBytesOp(const ByteBuffer *theBytes, AsyncSocket *theSocket = NULL) : SocketOp(theSocket), mBytesBuf(theBytes), mBytes(theBytes->data(), theBytes->length()) { mSocketEvent[SocketEvent_Write] = true; }
};

typedef SmartPtr<SendBytesOp> SendBytesOpPtr;

}; // namespace WONAPI

#endif
